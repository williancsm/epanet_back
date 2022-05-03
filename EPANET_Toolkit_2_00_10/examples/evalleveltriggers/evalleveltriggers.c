/*************************************************************************

 Example of EPANET Toolkit usage.

 evalpattern: evaluate a pump schedule pattern.

 ---------------------------------------------------------------------

                       Copyright (c) 2007
             Manuel Lopez-Ibanez <m.lopez-ibanez@napier.ac.uk>

 This program is free software (software libre); you can redistribute
 it and/or modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2 of the
 License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, you can obtain a copy of the GNU
 General Public License at:
                 http://www.gnu.org/copyleft/gpl.html
 or by writing to:
           Free Software Foundation, Inc., 59 Temple Place,
                 Suite 330, Boston, MA 02111-1307 USA

*************************************************************************/
#include "toolkit.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h> /* for fabs() */
#include <limits.h> /* for INT_MAX */

#define MIN(A,B)  (((A) < (B)) ? (A) : (B))

static int ENperror(const int e)
{
    char errmsg[EN_MAX_MSG_LEN+1];

    if (e > 100) {
        ENgeterror (e, errmsg, EN_MAX_MSG_LEN);
        fprintf (stderr, "EPANET:%s\n", errmsg);
        ENclose ();
        exit (EXIT_FAILURE);
    }
    return e;
}

void usage (void)
{
    printf("\n"
           "Usage: evalleveltriggers NETWORK.INP SCHEDULE_PATTERN\n\n");
    printf(
"Evaluate the schedule defined by the level-based triggers given in         \n"
"SCHEDULE_PATTERN on the network defined in NETWORK.INP"
"\n\n"
"Copyright (C) 2007 \n"
"Manuel Lopez-Ibanez (m.lopez-ibanez@napier.ac.uk)\n"
"\n"
"This is free software, and you are welcome to redistribute it under certain\n"
"conditions.  See the GNU General Public License for details. There is NO   \n"
"warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
"\n");

}

int main(int argc, char* argv[])
{
    FILE* schedule_file;

    int num_pumps, num_tanks, num_juncs, num_links, num_reservoirs;

    int i,j;

    float demand=0;
    float inflow=0;
    float totaltanks=0;
    float totalcost;
    int total_sw = 0, min_idletime = INT_MAX;

    char inp_filename[EN_MAX_FILENAME_LEN + 1];
    char schedule_filename[EN_MAX_FILENAME_LEN + 1];
    char rep_filename[EN_MAX_FILENAME_LEN + 1] = "/dev/null";/* report.rpt"; */
    char out_filename[EN_MAX_FILENAME_LEN + 1] = ""; /* "binary.rpt"; */

    char **pump_id;
    char **tank_id;
    int *pump_index;
    int *tank_index;
    float * tank_maxlevel;
    float * tank_minlevel;
    
    int offpeak_tariff_starts, peak_tariff_starts;
    char controllers[256];
    int *controlledby;
    float *canonical_schedule;

    if (argc < 3) {
        usage ();
        exit (EXIT_FAILURE);
    }

    strncpy(inp_filename, argv[1], EN_MAX_FILENAME_LEN);
    strncpy(schedule_filename, argv[2], EN_MAX_FILENAME_LEN);

    ENperror (ENopen (inp_filename, rep_filename, out_filename));

    ENgetcount(EN_LINKCOUNT,  &num_links);
    ENgetcount(EN_JUNCSCOUNT, &num_juncs);
    ENgetcount(EN_PUMPCOUNT, &num_pumps);
    ENgetcount(EN_TANKCOUNT, &num_tanks);
    ENgetcount(EN_RESERVCOUNT, &num_reservoirs);
    num_tanks = num_tanks - num_reservoirs;

    printf("Network: %s: %d links  %d junctions  %d pumps and  %d tanks\n\n",
           inp_filename, num_links, num_juncs, num_pumps, num_tanks);

    pump_id = malloc (sizeof(char *) * num_pumps);
    pump_index = malloc (sizeof(int) * num_pumps);

    for (i = 0; i < num_pumps; i++) {
	ENgetpumpindex (i+1, &pump_index[i]);
	pump_id[i] = malloc(sizeof(char) * EN_MAX_ID_LEN);
	ENgetlinkid (pump_index[i], pump_id[i]);
    }

    tank_id = malloc(sizeof(char*)*num_tanks);
    tank_index = malloc(sizeof(int)*num_tanks);
    for (i = 0; i < num_tanks; i++) {
	ENgettankindex (i+1, &tank_index[i]);
	tank_id[i] = malloc(sizeof(char) * EN_MAX_ID_LEN);
	ENgetnodeid (tank_index[i], tank_id[i]);
    }

    /* Tank limits.  */
    tank_maxlevel = malloc (sizeof(float) * num_tanks);
    tank_minlevel = malloc (sizeof(float) * num_tanks);
    for (i = 0; i < num_tanks; i++) {
        ENgetnodevalue (tank_index[i], EN_MAXLEVEL, &tank_maxlevel[i]);
        ENgetnodevalue (tank_index[i], EN_MINLEVEL, &tank_minlevel[i]);
    }

    /* Remove any utilisation patterns on pumps.  */
    for (i = 0; i < num_pumps; i++)
        ENsetlinkvalue (pump_index[i], EN_UPATTERN, 0);
    

    /* Read schedule.  */
    schedule_file = fopen (schedule_filename, "r");
    if (schedule_file == NULL) {
        perror (schedule_filename);
        exit (EXIT_FAILURE);
    }

    fscanf (schedule_file, "OFFPEAK_TARIFF_STARTS: %d\n", 
            &offpeak_tariff_starts);
    fscanf (schedule_file, "PEAK_TARIFF_STARTS: %d\n", 
            &peak_tariff_starts);
    fscanf (schedule_file, "CONTROLLERS: %255[^\n]\n", controllers);

    /* Parse CONTROLLERS.  */
    controlledby = malloc (num_pumps * sizeof(int));
    for (i = 0; i < num_pumps; i++) {
        char tmp_tank_id[EN_MAX_ID_LEN+1];
        char tmp_pump_id[EN_MAX_ID_LEN+1];
        int tmp_tank_index;
        char *substring;

        substring = strstr (controllers, pump_id[i]);
        if (substring == NULL) {
            fprintf (stderr, "error: pump %s is not controlled by any tank!\n",
                     pump_id[i]);
            exit (1);
        }
        sscanf (substring, "%[^: \t\n]:%[^: \t\n]", tmp_pump_id, tmp_tank_id);
#if DEBUG > 1
        fprintf (stderr,"%s:%s ", tmp_pump_id, tmp_tank_id);
#endif

        /* Look for the corresponding tank.  */
        tmp_tank_index = num_tanks;
        for (j = 0; j < num_tanks; j++) {
            if (strcmp (tank_id[j], tmp_tank_id) == 0) {
                tmp_tank_index = j;
                break;
            }
        }

        if (tmp_tank_index == num_tanks) {
            fprintf (stderr, "error: controller tank %s does not exist!\n",
                     tmp_tank_id);
            exit(1);
        }
        
        controlledby[i] = tmp_tank_index;
    }

    ENrulesclear ();


    /* Parse the trigger levels and assign them.  */
    for (i = 0; i < num_pumps; i++) {
        float lower_peak, upper_peak, lower_offpeak, upper_offpeak;
        if (fscanf (schedule_file, "%f %f %f %f", 
                    &lower_offpeak, &upper_offpeak, 
                    &lower_peak, &upper_peak) != 4) {
            fprintf (stderr, "error: can't convert input to floating-point\n");
            exit (1);
        }

#define epsilon 1e-5
#define check_trigger(LOWER, UPPER)                                          \
do {                                                                         \
    float __lower = LOWER;                                                   \
    float __upper = UPPER;                                                   \
    float __min   = tank_minlevel[controlledby[i]];                          \
    float __max   = tank_maxlevel[controlledby[i]];                          \
check_condition ((__lower > __min || fabs (__lower - __min) <= epsilon)      \
                 && (__lower < __max || fabs (__lower - __max) <= epsilon),  \
                "lower trigger out of limits");                              \
                                                                             \
check_condition ((__upper > __min || fabs (__upper - __min) <= epsilon)      \
                 && (__upper < __max || fabs (__upper - __max) <= epsilon),  \
                "upper trigger out of limits");                              \
                                                                             \
check_condition (__lower < __upper || fabs (__lower - __upper) <= epsilon,   \
                 "lower trigger higher than upper trigger");                 \
} while (0)                                
        
#define check_condition(COND, MSG)                                          \
do {                                                                        \
if (!(COND)) {                                                              \
    fprintf (stderr, "error: " MSG "\n"                                     \
             "pump = %s, tank = %s, triggers = [ %.16g, %.16g ], "          \
             "limits = [ %.16g, %.16g ]\n",                                 \
             pump_id[i], tank_id[controlledby[i]],                          \
             __lower, __upper, tank_minlevel[controlledby[i]],              \
             tank_maxlevel[controlledby[i]]);                               \
    exit (1);                                                               \
}                                                                           \
} while (0)

        check_trigger (lower_peak, upper_peak);
        check_trigger (lower_offpeak, upper_offpeak);
        
        /* setup peak tariff triggers. */
        ENperror (
            ENaddleveltrigger (pump_index[i], tank_index[controlledby[i]],
                               peak_tariff_starts, offpeak_tariff_starts,
                               lower_peak, 1));
        ENperror (
            ENaddleveltrigger (pump_index[i], tank_index[controlledby[i]],
                               peak_tariff_starts, offpeak_tariff_starts,
                               upper_peak, 0));
        /* setup off-peak tariff triggers.  */
        ENperror (
            ENaddleveltrigger (pump_index[i], tank_index[controlledby[i]],
                               offpeak_tariff_starts, peak_tariff_starts,
                               lower_offpeak, 1));
        ENperror (
            ENaddleveltrigger (pump_index[i], tank_index[controlledby[i]],
                               offpeak_tariff_starts, peak_tariff_starts,
                               upper_offpeak, 0));
    }
    
    /* Done parsing schedule. */
    fclose (schedule_file);

    /* Run hydraulic simulation */
    ENsolveH();

    printf("Pump: (Switches)[MinIdleTime]: Schedule\n");

    canonical_schedule = malloc (24 * sizeof (float));
    for (j = 0; j < 24; j++)
        canonical_schedule[j] = -1;
    
    for (i = 0; i < num_pumps; i++) {
        int sw, idletime;
        ENgetpumpswitches (pump_index[i], &sw);
	printf ("%s: (%8d)", pump_id[i], sw);
        total_sw += sw;

        ENgetminstoptime (pump_index[i], &idletime);
	printf ("[%11d]: ", idletime);

        if (idletime > 0)
            min_idletime = MIN (min_idletime, idletime);
        
        ENgetlinkvalue (pump_index[i], EN_SCHEDULE, canonical_schedule);
	for (j = 0; j < 24; j++) 
            printf (" %d", (int) canonical_schedule[j]);
	
	printf ("\n");
    }
    printf ("-------------------------------------------------------------\n");
    printf ("Total switches = %d, Min Idle Time = %d\n",
            total_sw, min_idletime);

    printf ("\n");
    printf (" Tank: Head(0) - Head = dHead : V(0) - Volume = dVolume\n");
    
    for (i = 0; i < num_tanks; i++) {
        float tanklevel, elevation, head, initvol, volume;

        ENgetnodevalue (tank_index[i], EN_TANKLEVEL, &tanklevel);
        ENgetnodevalue (tank_index[i], EN_ELEVATION, &elevation);
        ENgetnodevalue (tank_index[i], EN_HEAD, &head);
        ENgetnodevalue (tank_index[i], EN_INITVOL, &initvol);
        ENgetnodevalue (tank_index[i], EN_VOLUME, &volume);

	printf("%5s: %4.2f - %4.2f = %+.2f "
               ": %7.2f - %7.2f = %+6.2f\n",
               tank_id[i],
               tanklevel + elevation, head,
               tanklevel + elevation - head,
               initvol, volume, initvol - volume);
        totaltanks += initvol - volume;
    }

    printf ("\n");

    ENgettotaldemand(&demand);
    printf ("Total demand: % 10.2f\n", demand);

    ENgettotalinflow(&inflow);
    printf ("Total inflow: % 10.2f\n", inflow);
    printf ("             = % 10.4f\n", demand + inflow);
    printf ("Total tanks:  % 10.4f m^3\n", totaltanks);
    printf ("Difference = %.2f \n",
            totaltanks*1.0e+03 - (demand  + inflow));

    ENgettotalleakage(&inflow);
    printf("Total leakage: %.2f\n", inflow);

    printf ("\n");

    ENgettotalenergycost(&totalcost);
    printf ("Total energy cost:   % 10.2f\n", totalcost);


    ENclose();

    return 0;
}
