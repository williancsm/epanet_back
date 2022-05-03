/*************************************************************************

 Example of EPANET Toolkit usage.

 getcoords: get x,y coordinates of nodes in a network.

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

void usage (void)
{
    printf("\n"
           "Usage: getcoords NETWORK.INP\n\n");
    printf(
"Print x,y coordinates of all nodes in the network NETWORK.INP"
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
    char inp_filename[EN_MAX_FILENAME_LEN + 1];
    int num_nodes;
    int i;

    if (argc < 2) {
        usage ();
        exit (EXIT_FAILURE);
    }

    strncpy (inp_filename, argv[1], EN_MAX_FILENAME_LEN);

    ENopen (inp_filename, "/dev/null", "");

    ENgetcount(EN_NODECOUNT, &num_nodes);

    for(i=1; i <= num_nodes; i++) {
        float x, y;
        char nodeid[EN_MAX_ID_LEN + 1];

        ENgetnodeid(i, nodeid);
        ENgetnode_xcoord(i,&x);
        ENgetnode_ycoord(i,&y);
        fprintf(stdout, "%s\t%g\t%g\n", nodeid, x, y);
    }

    ENclose();

    return 0;
}
