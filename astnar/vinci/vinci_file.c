/****************************************************************************************/
/*                                                                                      */
/*                                    vinci_file.c                                      */
/*                                                                                      */
/****************************************************************************************/
/*                                                                                      */
/* Authors: Benno Bueeler (bueeler@ifor.math.ethz.ch)                                   */
/*          and                                                                         */
/*          Andreas Enge (enge@ifor.math.ethz.ch)                                       */
/*          Institute for Operations Research                                           */
/*	    Swiss Federal Institute of Technology Zurich                                     */
/*	    Switzerland                                                                      */
/*                                                                                      */
/* Last Changes: May 19, 2003                                                           */
/*                                                                                      */
/****************************************************************************************/
/*                                                                                      */
/* Modifications: This file, originally due to Benno Bueeler and Andreas Enge, is       */
/* modified for compatibility with ASTNAR.                                              */
/* Authors of the modification: Raven Beutner                                           */
/* Date of the last modification: April, 2021                                           */
/*                                                                                      */
/****************************************************************************************/
/*                                                                                      */
/* functions allowing to read data files in the format of Avis and Fukuda               */
/*                                                                                      */
/****************************************************************************************/

#include "vinci.h"

/****************************************************************************************/

FILE * open_read (char filename [255])
   /* opens the specified file, reads all lines up to a line 'begin' and returns a      */
   /* pointer to the file */
   
{  FILE    *fp;
   char    line [255];
   boolean begin_found = FALSE;
   

   if (! (fp = fopen (filename, "r")))
   {  fprintf (stderr, "\n***** ERROR: Could not open file '%s' in 'open_read'.\n", filename);
      exit (0);
   };
   while (!feof (fp) && !begin_found)
   {  fgets (line, 255, fp);
      if (!strncmp (line, "begin", 5)) begin_found = TRUE;
   };
   if (!begin_found)
   {  fprintf (stderr, "\n***** ERROR: File '%s' in 'open_read does not contain a line 'begin'.\n",
              filename);
      exit (0);
   };
   return fp;
}
   
/****************************************************************************************/

int determine_data_type (char *data_type)
   /* returns the integer code for the data type written in "data_type"                 */
   
{
   if (!strcmp (data_type, "integer"))
      return INTEGER_T;
   else if (!strcmp (data_type, "real"))
      return REAL_T;
   else if (!strcmp (data_type, "rational"))
      return RATIONAL_T;
   else
   {  fprintf (stderr, "\n***** ERROR: '%s' is not a known data type; use integer, real or rational.",
              data_type);
      return NONE;
   }
}

/****************************************************************************************/

void sread_rational_value (char *s, rational *value)
   /* reads a rational value from the specified string "s" and assigns it to "value"    */

{
   char     *numerator_s, *denominator_s = NULL, *position, token;
   int      sign = 1, i;
   rational numerator, denominator;

   /* determine the sign of the number */
   numerator_s = s;
   if (s [0] == '-')
   {  sign = -1;
      numerator_s++;
   }
   else if (s [0] == '+')
      numerator_s++;

   /* look for a sign '/' and in this case split the number in numerator and denominator */
   position = strchr (numerator_s, '/');
   if (position != NULL)
   {  *position = '\0'; /* terminates the numerator */
      denominator_s = position + 1;
   };

   /* determine the floating point values of numerator and denominator */
   numerator = 0;
   for (i = 0; i < strlen (numerator_s); i++)
   {  token = numerator_s [i];
      if (strchr ("0123456789", token)) /* token is a cypher */
         numerator = 10 * numerator + (int) token - 48;
   }

   if (position != NULL)
   {  denominator = 0;
      for (i = 0; i < strlen (denominator_s); i++)
      {  token = denominator_s [i];
         if (strchr ("0123456789", token)) /* token is a cypher */
            denominator = 10 * denominator + (int) token - 48;
      }
   }
   else denominator = 1;

   *value = sign * numerator / denominator;
}

/****************************************************************************************/

static void fread_rational_value (FILE *f, real *value)
   /* reads a rational value from the specified file "f" and assigns it to "value"      */

{
   char     number_s [255];
   rational rational_value;

   fscanf (f, "%s ", number_s);
   sread_rational_value (number_s, &rational_value);
   *value = rational_value;

}

/****************************************************************************************/

void read_hyperplanes (char filename [255])
{  FILE       *f;
   int        i, j;
   char       data_type_string [255];
   int        data_type;

   f = open_read (filename);
   fscanf (f, "%i %i %s ", &G_m, &G_d, data_type_string);

   G_d --;
   data_type = determine_data_type (data_type_string);
   if (data_type == RATIONAL_T)
   {  fprintf (stderr, "\n***** WARNING: The planes file is of rational type; all ");
      fprintf (stderr, "coordinates will be\ntransformed to floating point values.\n");
   }

   create_hyperplanes ();

   if (data_type == REAL_T)
      for (i = 0; i < G_m; i++)
      {  fscanf (f, "%le ", &(G_Hyperplanes [i] [G_d]));
         for (j = 0; j < G_d; j++)
         {  fscanf (f, "%le ", &(G_Hyperplanes [i] [j]));
            G_Hyperplanes [i] [j] = - G_Hyperplanes [i] [j];
         }
      }
   else
      for (i = 0; i < G_m; i++)
      {  fread_rational_value (f, &(G_Hyperplanes [i] [G_d]));
         for (j = 0; j < G_d; j++)
         {  fread_rational_value (f, &(G_Hyperplanes [i] [j]));
            G_Hyperplanes [i] [j] = - G_Hyperplanes [i] [j];
         }
      }

   fclose (f);
}

/****************************************************************************************/
