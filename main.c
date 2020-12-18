
/*********************************************************************************************

    This is public domain software that was developed by or for the U.S. Naval Oceanographic
    Office and/or the U.S. Army Corps of Engineers.

    This is a work of the U.S. Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the U.S. Government.

    Neither the United States Government, nor any employees of the United States Government,
    nor the author, makes any warranty, express or implied, without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.

*********************************************************************************************/

/* Jan C. Depner 04/03/00 - Sets the IHO (CLASS) flags in the gsf file. */

#include <stdio.h>
#include <string.h>

#include "nvutility.h"

#include "gsf.h"
#include "version.h"

extern int32_t gsfError;


int32_t main (int32_t argc, char **argv)
{
    gsfDataID           id;
    gsfRecords          gsfrecord;
    int32_t             hnd, j, min_class1_angle, max_class1_angle, 
                        min_class2_angle, max_class2_angle, recnum, 
                        percent = 0, old_percent = -1, ret;
    char                comment[16384];

                       
    int32_t write_history (int32_t, char **, char *, char *, int32_t);


    printf("\n\n %s \n\n",VERSION);


    if (argc < 6)
    {
        fprintf(stderr, "USAGE: gsf_class GSF_FILE MIN_CLASS2_ANGLE MIN_CLASS1_ANGLE MAX_CLASS1_ANGLE MAX_CLASS2_ANGLE\n");
        exit(-1);
    }

    sscanf (argv[2], "%d", &min_class2_angle);
    sscanf (argv[3], "%d", &min_class1_angle);
    sscanf (argv[4], "%d", &max_class1_angle);
    sscanf (argv[5], "%d", &max_class2_angle);


    if (gsfOpen (argv[1], GSF_UPDATE_INDEX, &hnd))
    {
        gsfPrintError(stderr);
        exit (-1);
    }

    printf("File : %s\n\n", argv[1]);

    recnum=0;
    while (1)
    {
        id.recordID = GSF_RECORD_SWATH_BATHYMETRY_PING;
        id.record_number = recnum;
        if (gsfRead (hnd, GSF_RECORD_SWATH_BATHYMETRY_PING, &id, &gsfrecord, 
            NULL, 0) < 0)
        {
            if (gsfError == GSF_INVALID_RECORD_NUMBER) break;
        }


        for (j = 1 ; j <= gsfrecord.mb_ping.number_beams ; j++) 
        {
            /*  Check for em1000 (no beam angles).  */

            if (strstr (argv[1], "mbb"))
            {
                if (j - 1 < min_class2_angle || 
                    j - 1 > max_class2_angle)
                {
                    gsfrecord.mb_ping.beam_flags[j - 1] = 
                        NV_GSF_IGNORE_NOT_2X_IHO;
                }
                else if (j - 1 < min_class1_angle || 
                    j - 1 > max_class1_angle)
                {
                    gsfrecord.mb_ping.beam_flags[j - 1] |= 
                        NV_GSF_INFO_NOT_1X_IHO;
                }
            }
            else
            {
                if (gsfrecord.mb_ping.beam_angle[j - 1] < min_class2_angle || 
                    gsfrecord.mb_ping.beam_angle[j - 1] > max_class2_angle)
                {
                    gsfrecord.mb_ping.beam_flags[j - 1] = 
                        NV_GSF_IGNORE_NOT_2X_IHO;
                }
                else if (gsfrecord.mb_ping.beam_angle[j - 1] < 
                    min_class1_angle || gsfrecord.mb_ping.beam_angle[j - 1] > 
                    max_class1_angle)
                {
                    gsfrecord.mb_ping.beam_flags[j - 1] |= 
                        NV_GSF_INFO_NOT_1X_IHO;
                }
            }
        }

        id.recordID = GSF_RECORD_SWATH_BATHYMETRY_PING;
        id.record_number = recnum;
        if (gsfWrite(hnd, &id, &gsfrecord) < 0)
        {
            gsfPrintError(stderr);
            exit(-1);
        }
           

        recnum++;


        percent = gsfPercent(hnd);
        if ( old_percent != percent )
        {
            printf ("%3d%% processed    \r", percent);
            fflush (stdout);
            old_percent = percent;
        }
    }


    percent = 100;
    printf ("%3d%% processed    \n", percent);
        
    gsfClose(hnd);
    printf("\n");


    /*  Open the file non-indexed so that we can write a history record.  */

    if (gsfOpen (argv[1], GSF_UPDATE, &hnd))
    {
        gsfPrintError (stderr);
        exit (-1);
    }


    /*  Write a history record describing the angles.  */

    sprintf (comment, 
        "GSF Beam Classes:\nClass 2 min angle - %s\nClass 2 max angle - %s\nClass 1 min angle - %s\nClass 1 max angle - %s\n", 
        argv[2], argv[5], argv[3], argv[4]);


    ret = write_history (argc, argv, comment, argv[1], hnd);
    if (ret)
    {
        fprintf(stderr, "Error: %d - writing gsf history record\n", ret);
        fflush (stderr);
    }

    gsfClose (hnd);


    /*  Please ignore the following line.  It is useless.  Except...

        On some versions of Ubuntu, if I compile a program that doesn't use the math
	library but it calls a shared library that does use the math library I get undefined
	references to some of the math library functions even though I have -lm as the last
	library listed on the link line.  This happens whether I use qmake to build the
	Makefile or I have a pre-built Makefile.  Including math.h doesn't fix it either.
	The following line forces the linker to bring in the math library.  If there is a
	better solution please let me know at area.based.editor AT gmail DOT com.  */

    float ubuntu; ubuntu = 4.50 ; ubuntu = fmod (ubuntu, 1.0);

    return (0);
}
