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
#include "epanet2.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <limits.h> /* for INT_MAX */

#define MIN(A,B)  (((A) < (B)) ? (A) : (B))

static int ENperror(const int e)
{
    char errmsg[EN_MAXMSG+1];

    if (e > 100) {
        ENgeterror (e, errmsg, EN_MAXMSG);
        fprintf (stderr, "EPANET:%s\n", errmsg);
        ENclose ();
        exit (EXIT_FAILURE);
    }
    return e;
}

void usage (void)
{
    printf("\n"
           "Usage: evalpattern NETWORK.INP SCHEDULE_PATTERN\n\n");
    printf(
"Evaluate a binary pattern schedule given in SCHEDULE_PATTERN \n"
"on the network defined in NETWORK.INP"
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
    FILE* pumps_file;

    int num_pumps, num_tanks, num_juncs, num_links, num_reservoirs;

    int pump_status, hour,p, i,j;

    float demand=0;
    float inflow=0;
    float totaltanks=0;
    float totalcost;
    int total_sw = 0, min_idletime = INT_MAX;

    char inp_filename[EN_MAXMSG + 1];
    char pumps_filename[EN_MAXMSG + 1];
    char rep_filename[EN_MAXMSG + 1] = "/dev/null";/* report.rpt"; */
    char out_filename[EN_MAXMSG + 1] = ""; /* "binary.rpt"; */

    char **pump_id;
    char **tank_id;
    int *pump_index;
    int *tank_index;
    int *pattern_index;
    float **patterns;
    

    if (argc < 3) {
        usage ();
        exit (EXIT_FAILURE);
    }

    strncpy(inp_filename, argv[1], EN_MAXMSG);
    strncpy(pumps_filename, argv[2], EN_MAXMSG);

    ENperror (ENopen (inp_filename, rep_filename, out_filename));

    ENgetcount(EN_LINKCOUNT,  &num_links);
    //ENgetcount(EN_JUNCSCOUNT, &num_juncs);
    //ENgetcount(EN_PUMPCOUNT, &num_pumps);
    ENgetcount(EN_TANKCOUNT, &num_tanks);
    //ENgetcount(EN_RESERVCOUNT, &num_reservoirs);
    num_tanks = num_tanks - num_reservoirs;

    printf("Network: %s: %d links  %d junctions  %d pumps and  %d tanks\n\n",
           inp_filename, num_links, num_juncs, num_pumps, num_tanks);

    pump_id = malloc (sizeof(char *) * num_pumps);
    pump_index = malloc (sizeof(int) * num_pumps);

    for (i = 0; i < num_pumps; i++) {
	ENgetpumpindex (i+1, &pump_index[i]);
	pump_id[i] = malloc(sizeof(char) * EN_MAXID);
	ENgetlinkid (pump_index[i], pump_id[i]);
    }

    tank_id = malloc(sizeof(char*)*num_tanks);
    tank_index = malloc(sizeof(int)*num_tanks);
    for (i = 0; i < num_tanks; i++) {
	//ENgettankindex (i+1, &tank_index[i]);
	tank_id[i] = malloc(sizeof(char) * EN_MAXID);
	ENgetnodeid (tank_index[i], tank_id[i]);
    }

    pattern_index = malloc(sizeof(int)*num_pumps);
    patterns = malloc(sizeof(float*)*num_pumps);
    for (i = 0; i < num_pumps; i++) {
	patterns[i] = malloc(sizeof(float)*24);
    }

    for (i = 0; i < num_pumps; i++) {
        ENperror (ENaddpattern (pump_id[i]));
        ENperror (ENgetpatternindex (pump_id[i], &pattern_index[i]));
    }

    /* Read pump schedule patterns */
    pumps_file = fopen(pumps_filename, "r");
    if (pumps_file == NULL) {
        perror (pumps_filename);
        exit (EXIT_FAILURE);
    }

    do {
        pump_status = fgetc(pumps_file);
    } while (isspace(pump_status));

    p = 0;

    while (pump_status != EOF) {
        if (p >= num_pumps) {
            fprintf (stderr, "warning: "
                     "'%s' contains more patterns than pumps in '%s'\n", 
                     pumps_filename, inp_filename);
            break;
        }
	hour=0;
	while (pump_status == '0' || pump_status=='1') {
	    if(pump_status == '0') {
		patterns[p][hour] = 0.0;
	    }
	    else {
		patterns[p][hour] = 1.0;
	    }
	    hour++;
	    pump_status = fgetc(pumps_file);
	}
	ENperror (ENsetpattern (pattern_index[p], patterns[p], 24));
	ENperror (ENgetlinkindex (pump_id[p], &pump_index[p]));
	ENperror (ENsetlinkvalue (pump_index[p], EN_LINKPATTERN,
                                  pattern_index[p]));
	p++;
	while (isspace(pump_status))
	    pump_status = fgetc(pumps_file);
    }
    fclose(pumps_file);

    /* Run hydraulic simulation */
    ENsolveH();

    printf("Pump: (Switches)[MinIdleTime]: Schedule\n");

    for (i = 0; i < num_pumps; i++) {
        int sw, idletime;
        ENgetpumpswitches (pump_index[i], &sw);
	printf ("%s: (%8d)", pump_id[i], sw);
        total_sw += sw;

        ENgetminstoptime (pump_index[i], &idletime);
	printf ("[%11d]: ", idletime);

        if (idletime > 0)
            min_idletime = MIN (min_idletime, idletime);
        
	for (j = 0; j < 24; j++) {
            float value;
            ENgetpatternvalue (pattern_index[i], j+1, &value);
            printf ("%d", (int) value);
	}
	printf ("\n");
    }
    printf ("-------------------------------------------------------------\n");
    printf ("Total switches = %d, Min Idle Time = %d\n",
            total_sw, min_idletime);

    printf ("\n");

    printf (" Tank: Head(0) - Head = dHead : V(0) - Volume = dVolume\n");
    
    for(i=0; i < num_tanks; i++) {
        float tanklevel, elevation, head, initvol, volume;

        ENgetnodevalue (tank_index[i], EN_TANKLEVEL, &tanklevel);
        ENgetnodevalue (tank_index[i], EN_ELEVATION, &elevation);
        ENgetnodevalue (tank_index[i], EN_HEAD, &head);
        ENgetnodevalue (tank_index[i], EN_INITVOLUME, &initvol);
        ENgetnodevalue (tank_index[i], EN_TANKVOLUME, &volume);

	printf("%5s: %4.2f - %4.2f = %+.2f "
               ": %7.2f - %7.2f = %+6.2f\n",
               tank_id[i],
               tanklevel + elevation, head,
               tanklevel + elevation - head,
               initvol, volume, initvol - volume);
        totaltanks += initvol - volume;
    }

    printf ("\n");

    //ENgettotaldemand(&demand);
    printf ("Total demand: % 10.2f\n", demand);

    //ENgettotalinflow(&inflow);
    printf ("Total inflow: % 10.2f\n", inflow);
    printf ("             = % 10.4f\n", demand + inflow);
    printf ("Total tanks:  % 10.4f m^3\n", totaltanks);
    printf ("Difference = %.2f \n",
            totaltanks*1.0e+03 - (demand  + inflow));

    //ENgettotalleakage(&inflow);
    printf("Total leakage: %.2f\n", inflow);

    printf ("\n");

    //ENgettotalenergycost(&totalcost);
    printf ("Total energy cost:   % 10.2f\n", totalcost);


    ENclose();

    return 0;
}
