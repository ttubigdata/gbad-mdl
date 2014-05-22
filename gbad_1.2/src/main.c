//---------------------------------------------------------------------------
// main.c
//
// SUBDUE, versions 5.1.2 - GBAD, version 1.2
//---------------------------------------------------------------------------

#include "subdue.h"
#include "time.h"
#include <unistd.h>
#include <sys/times.h>

// Function prototypes

int main(int, char **);
void ISubdue(Parameters *);
Parameters *GetParameters(int, char **);
void PrintParameters(Parameters *);
void FreeParameters(Parameters *);
Parameters *PostProcessParameters(Parameters *parameters);

//---------------------------------------------------------------------------
// NAME:    main
//
// INPUTS:  (int argc) - number of arguments to program
//          (char **argv) - array of strings of arguments to program
//
// RETURN:  (int) - 0 if all is well
//
// PURPOSE: Main Subdue function that processes command-line arguments
//          and initiates discovery.
//---------------------------------------------------------------------------

int main(int argc, char **argv)
{
   struct tms tmsstart, tmsend;
   clock_t startTime, endTime;
   static long clktck = 0;
   time_t iterationStartTime;
   time_t iterationEndTime;
   SubList *subList;
   Parameters *parameters;
   FILE *outputFile;
   ULONG iteration;
   BOOLEAN done;
   // GBAD: Variables for creating new sublist and for saving evaluation method.
   SubList *newSubList;
   SubList *mdlSubList;
   ULONG saveEvalMethod;
   // GBAD

   clktck = sysconf(_SC_CLK_TCK);
   startTime = times(&tmsstart);
   printf("GBAD/Subdue %s\n\n", SUBDUE_VERSION);
   parameters = GetParameters(argc, argv);

   if (parameters->incremental)
      ISubdue(parameters);
   else
   {
      // compress pos and neg graphs with predefined subs, if given
      if (parameters->numPreSubs > 0)
         CompressWithPredefinedSubs(parameters);

      PostProcessParameters(parameters);
      PrintParameters(parameters);

      if (parameters->iterations > 1)
         printf("----- Iteration 1 -----\n\n");

      iteration = 1;
      //
      // GBAD-P: Set current iteration.
      //
      parameters->currentIteration = iteration;
      //
      // GBAD-P
      //
      done = FALSE;
      while ((iteration <= parameters->iterations) && (!done))
      {
         iterationStartTime = time(NULL);
         if (iteration > 1)
            printf("----- Iteration %lu -----\n\n", iteration);

         printf("%lu positive graphs: %lu vertices, %lu edges",
                parameters->numPosEgs, parameters->posGraph->numVertices,
                parameters->posGraph->numEdges);

         if (parameters->evalMethod == EVAL_MDL)
            printf(", %.0f bits\n", parameters->posGraphDL);
         else
            printf("\n");
         if (parameters->negGraph != NULL) 
         {
            printf("%lu negative graphs: %lu vertices, %lu edges",
                   parameters->numNegEgs, parameters->negGraph->numVertices,
                   parameters->negGraph->numEdges);
            if (parameters->evalMethod == EVAL_MDL)
               printf(", %.0f bits\n", parameters->negGraphDL);
            else 
               printf("\n");
         }
         printf("%lu unique labels\n", parameters->labelList->numLabels);
         printf("\n");

         //
         // GBAD-P: Added iteration parameter.
         //
         if ((parameters->prob) && (iteration > 1))
         {
            // If GBAD-P option chosen, after the first iteration, we no longer
            // care about minsize of maxsize after the first iteration (if the
            // user specified these parameters), as we are just dealing with
            // single extensions from the normative - so set it to where we
            // just look at substructures that are composed of the normative
            // pattern (SUB_) and the single vertex extension.
            parameters->minVertices = 1;
            parameters->maxVertices = 2;
//////////////////////////////////////////////////// WFE - FIX
            //
	    // -prune is useful to get to the normative pattern, but possibly
	    // detremental to discovering anomalies... so, turn off pruning
	    // (in case it was turned on), so that it is not used in future
	    // iterations.
	    //
	    parameters->prune = FALSE;
//////////////////////////////////////////////////// WFE - FIX
         }
	 //
	 // GBAD-P
	 //
         subList = DiscoverSubs(parameters, iteration);
         //

         if (subList->head == NULL) 
         {
            done = TRUE;
            printf("No substructures found.\n\n");
         }
         else 
         {
            //
            // GBAD-MDL
            //
            if ((parameters->mdl) && (!parameters->prob) && (!parameters->mps))
            {
               //newSubList = FindTrueBestSubstructure(subList, parameters);
	       //
	       // NOTE: FindAnomalousSubstructure should probably be renamed
	       //       to FindAnomalousInstances, because it returns a
	       //       substructure list of ONE substructure (the normative
	       //       pattern), with all of the anomalous instances
	       //       attached.
	       //
	       newSubList = FindAnomalousSubstructure(subList, parameters);
	       if (newSubList != NULL)
                  PrintAnomalousInstances(newSubList,parameters);
	       else
	          printf("Anomalous Instances:  NONE\n");
            }
            //
            // GBAD-P
            //
	    // NOTE: We are only applying the probabilistic approach to the
	    //       substructures AFTER the first iteration, as the first
	    //       iteration would not contain any extensions.
	    //
            if ((parameters->prob) && (!parameters->mdl) && (!parameters->mps) && 
	        (iteration > 1))
            {
               //
               // Now that we have the best substructure, do the following:
               // 1. create a list of extended instances from best substructure
               //    instances.
               // 2. print the instances of lowest probability.
               //
               // If a different evaluation metric was used to find the best
               // substructure, change the evaluation technique for finding the
               // anomalous instance(s).
               //
               saveEvalMethod = parameters->evalMethod;
               parameters->evalMethod = EVAL_MDL;
               newSubList = ModifyBestSubstructureList(subList, parameters);
               //
               // If there is not a new substructure list, that means no anomalies
               // can be found, so skip the rest of this logic
               //
               if (newSubList != NULL)
               {
                  subList = CopySubList(newSubList,parameters);
                  if (subList->head->sub->instances != NULL)
                     PrintProbabilisticAnomalies(subList->head->sub->instances,parameters);
               }
               //
               // If a different evaluation metric was used to find the best
               // substructure, change the evaluation technique for finding the
               // anomalous instance(s)
               //
               parameters->evalMethod = saveEvalMethod;
            } else if ((parameters->prob) && (!parameters->mps)) {
               printf("Normative Pattern:\n");
               PrintSub(subList->head->sub,parameters);
               printf("\n");
            }
            //
            // GBAD-MPS
            //
            if ((parameters->mps) && (iteration == 1))
            {
	       //
	       // GBAD-MDL:  If MDL approach also specified, need to find the
	       //            substructure that deviates by the specified MDL
	       //            threshold.
	       //
               //
               // Now, we call PrintMaxPartialAnomalousInstances, which compares
               // the parent instances to the best instances (from the best
               // substructure), and outputs the ones that did not extend to
               // the instances from the best substructure.
               //
               // NOTE: With this approach, the Print routines has two 
               //       purposes - print the anomaly AND reset the best 
               //       substructure for possible multiple iterations.
               //
	       if (parameters->mdl)
	       {
                  mdlSubList = FindAnomalousSubstructure(subList, parameters);
                  printf("\nNormative Pattern (from MDL algorithm):\n");
                  PrintSub(mdlSubList->head->sub,parameters);
                  printf("\n");
                  newSubList = PrintMaxPartialAnomalousInstances(mdlSubList, parameters);
	       } else {
                  printf("\nNormative Pattern:\n");
                  PrintSub(subList->head->sub,parameters);
                  printf("\n");
                  newSubList = PrintMaxPartialAnomalousInstances(subList, parameters);
	       }

               if ((newSubList != NULL) && (CountSubs(newSubList) > 0))
               {
                  subList = CopySubList(newSubList,parameters);
               }
	       //
	       // If more than one iteration was specified, turn off the MPS
	       // and MDL approaches for subsequent iterations (just use the 
	       // probabilistic approach, as the probabilistic approach is the
	       // only appropriate algorithm for examining single extensions).
	       //
	       if (parameters->iterations > 1) {
	          parameters->mps = FALSE;
	          parameters->mdl = FALSE;
	          parameters->prob = TRUE;
	       }
            }
            //
	    // GBAD
	    //

            // write output to stdout
            if (parameters->outputLevel > 1) 
            {
               printf("\nBest %lu substructures:\n\n", CountSubs (subList));
               PrintSubList(subList, parameters);
            } 
            else 
            {
               printf("\nBest substructure: ");
	       if ((CountSubs(subList) > 0) && (subList->head->sub != NULL))
                  PrintSub(subList->head->sub, parameters);
	       else
	          printf("None.");
	       printf("\n\n");
            }

            // write machine-readable output to file, if given
            if (parameters->outputToFile) 
            {
               outputFile = fopen(parameters->outFileName, "a");
               if (outputFile == NULL) 
               {
                  printf("WARNING: unable to write to output file %s,",
                         parameters->outFileName);
                  printf("disabling\n");
                  parameters->outputToFile = FALSE;
               }
               WriteGraphToFile(outputFile, subList->head->sub->definition,
                                parameters->labelList, 0, 0,
                                subList->head->sub->definition->numVertices,
                                TRUE);
               fclose(outputFile);
            }

            if (iteration < parameters->iterations) 
            {                                    // Another iteration?
               if (parameters->evalMethod == EVAL_SETCOVER) 
               {
                  printf("Removing positive examples covered by");
                  printf(" best substructure.\n\n");
                  RemovePosEgsCovered(subList->head->sub, parameters);
               } 
               else 
                  CompressFinalGraphs(subList->head->sub, parameters, iteration,
                                      FALSE);

               // check for stopping condition
               // if set-covering, then no more positive examples
               // if MDL or size, then positive graph contains no edges
               if (parameters->evalMethod == EVAL_SETCOVER) 
               {
                  if (parameters->numPosEgs == 0) 
                  {
                     done = TRUE;
                     printf("Ending iterations - ");
                     printf("all positive examples covered.\n\n");
                  }
               } 
               else 
               {
                  if (parameters->posGraph->numEdges == 0) 
                  {
                     done = TRUE;
                     printf("Ending iterations - graph fully compressed.\n\n");
                  }
               }
            }
            if ((iteration == parameters->iterations) && (parameters->compress))
            {
               if (parameters->evalMethod == EVAL_SETCOVER)
                  WriteUpdatedGraphToFile(subList->head->sub, parameters);
               else 
                  WriteCompressedGraphToFile(subList->head->sub, parameters,
                                             iteration);
            }
         }
         FreeSubList(subList);
         if (parameters->iterations > 1) 
         {
            iterationEndTime = time(NULL);
            printf("Elapsed time for iteration %lu = %lu seconds.\n\n",
            iteration, (iterationEndTime - iterationStartTime));
         }
         iteration++;
         //
         // GBAD-P: Set current iteration.
         //
         parameters->currentIteration = iteration;
         //
	 // GBAD-P
	 //
      }
   }
 
   FreeParameters(parameters);
   endTime = times(&tmsend);
   printf("\nGBAD/Subdue done (elapsed CPU time = %7.2f seconds).\n",
          (endTime - startTime) / (double) clktck);
   return 0;
}


//---------------------------------------------------------------------------
// NAME:   ISubdue 
//
// INPUTS: (Parameters *parameters)
//
// RETURN: void
//
// PURPOSE: Perform incremental discovery.
//---------------------------------------------------------------------------

void ISubdue(Parameters *parameters)
{
   FILE *outputFile;
   BOOLEAN done;
   BOOLEAN ignoreBoundary = FALSE;
   BOOLEAN newData = FALSE;
   ULONG incrementCount = 0;
   ULONG sizeOfPosIncrement;
   ULONG sizeOfNegIncrement;
   Increment *increment;
   InstanceList *boundaryInstances = NULL;
   SubList *localSubList = NULL;
   SubList *globalSubList = NULL;

   if (parameters->evalMethod == EVAL_SETCOVER)
      ignoreBoundary = TRUE;

   while(TRUE)
   {
      // Get next batch of data, either dependent or independent
      newData = GetNextIncrement(parameters);
      if (!newData)
         break;
 
      PostProcessParameters(parameters);
      PrintParameters(parameters);
 
      increment = GetCurrentIncrement(parameters);
      parameters->posGraphSize +=
         (increment->numPosVertices + increment->numPosEdges);
   
      if (parameters->evalMethod == EVAL_SETCOVER)
         SetIncrementNumExamples(parameters);
      else
      {
         // We have to set the size here instead of in evaluate, otherwise
         // it gets reset at each iteration to the compressed graph size
         sizeOfPosIncrement =
            IncrementSize(parameters, GetCurrentIncrementNum(parameters), POS);
         sizeOfNegIncrement =
            IncrementSize(parameters, GetCurrentIncrementNum(parameters), NEG);
      }

      printf("Increment #%lu: %lu positive vertices, %lu positive edges\n",
         incrementCount+1, increment->numPosVertices, increment->numPosEdges);
      printf("Accumulated Positive Graph Size: %lu vertices, %lu edges\n",
         parameters->posGraph->numVertices, parameters->posGraph->numEdges);
      if (parameters->negGraph != NULL)
      {
         printf("Increment #%lu: %lu negative vertices, %lu negative edges\n",
           incrementCount+1, increment->numNegVertices, increment->numNegEdges);
         printf("Accumulated Negative Graph Size: %lu vertices, %lu edges\n",
            parameters->negGraph->numVertices, parameters->negGraph->numEdges);
      }
   
      done = FALSE;
      printf("%lu unique labels\n", parameters->labelList->numLabels);
      printf("\n");

      // Return the n best subs for this increment
      //
      // GBAD-P: Added iteration parameter.
      //
      localSubList = DiscoverSubs(parameters, 0);
      //
      // GBAD-P
      //
      if (localSubList->head == NULL)
      {
         printf("No local substructures found.\n\n");
         done = TRUE;
      }
      else
      {
         if (parameters->outputLevel > 1)
         {
            // print local subs
            printf("\n");
            printf("Best %lu local substructures ", CountSubs(localSubList));
            printf("before boundary processing:\n");
            PrintSubList(localSubList, parameters);
         }
                                        // Compress only supports EVAL_SETCOVER 
         if (parameters->compress)      // Compress using best and write to file
         {
            printf("Removing positive examples covered by");
            printf(" best substructure.\n\n");
            WriteUpdatedIncToFile(localSubList->head->sub, parameters);
         }

         // Store a copy of the local subs in the current increment data
         // we only keep the instance list for the current and
         // immediate predecessor increments
         StoreSubs(localSubList, parameters);

         // Compute the best overall subs for all increments,
         // before boundary evaluation
         // We do this so we have a more accurate list of best subs for the
         // boundary evaluation, although it is not technically necessary
         globalSubList = ComputeBestSubstructures(parameters, 0);
         if (!ignoreBoundary)
         {
            if (globalSubList != NULL)
            {
               boundaryInstances =
                  EvaluateBoundaryInstances(globalSubList, parameters);
               FreeSubList(globalSubList);
            } 

            if (parameters->outputLevel > 1)
            {
               if ((boundaryInstances != NULL) &&
                   (boundaryInstances->head != NULL))
               {
                  printf("Boundary instances found:\n");
                  PrintInstanceList(boundaryInstances, parameters->posGraph,
                                    parameters->labelList);
               }
               else 
                  printf("No Boundary Instances Were Found.\n");

               printf("\n");
               printf("Best %lu local substructures ",
                      CountSubs(increment->subList));
               printf("after boundary evaluation:\n");
               PrintStoredSubList(increment->subList, parameters);
            }

            // Recompute the globally best subs after the boundary instances
            // have been recovered
            globalSubList = ComputeBestSubstructures(parameters, 0);
         }
         printf("\nGlobally Best Substructures - Final:\n");
         PrintStoredSubList(globalSubList, parameters);
         FreeSubList(globalSubList);
      }

      // write machine-readable output to file, if given
      if (parameters->outputToFile)
      {
         outputFile = fopen(parameters->outFileName, "a");
         if (outputFile == NULL)
         {
            printf("WARNING: unable to write to output file %s, disabling\n",
                   parameters->outFileName);
            parameters->outputToFile = FALSE;
         }
         if (parameters->evalMethod == EVAL_SETCOVER)
	 {
            printf("Removing positive examples covered by");
            printf(" best substructure.\n\n");
            RemovePosEgsCovered(localSubList->head->sub, parameters);
            WriteGraphToFile(outputFile, localSubList->head->sub->definition,
                             parameters->labelList, 0, 0,
                             localSubList->head->sub->definition->numVertices,
                             TRUE);
         }
	 else 
            WriteGraphToFile(outputFile, localSubList->head->sub->definition,
                             parameters->labelList, 0, 0,
                             localSubList->head->sub->definition->numVertices,
                             TRUE);
         fclose(outputFile);
      }
      incrementCount++;
   }
}


//---------------------------------------------------------------------------
// NAME: GetParameters
//
// INPUTS: (int argc) - number of command-line arguments
//         (char *argv[]) - array of command-line argument strings
//
// RETURN: (Parameters *)
//
// PURPOSE: Initialize parameters structure and process command-line
//          options.
//---------------------------------------------------------------------------

Parameters *GetParameters(int argc, char *argv[])
{
   Parameters *parameters;
   int i;
   double doubleArg;
   ULONG ulongArg;
   BOOLEAN limitSet = FALSE;
   FILE *outputFile;

   parameters = (Parameters *) malloc(sizeof(Parameters));
   if (parameters == NULL)
      OutOfMemoryError("parameters");

   // initialize default parameter settings
   parameters->directed = TRUE;
   parameters->limit = 0;
   parameters->numBestSubs = 3;
   parameters->beamWidth = 4;
   parameters->valueBased = FALSE;
   parameters->prune = FALSE;
   strcpy(parameters->outFileName, "none");
   parameters->outputToFile = FALSE;
   parameters->outputLevel = 2;
   parameters->allowInstanceOverlap = FALSE;
   parameters->threshold = 0.0;
   parameters->evalMethod = EVAL_MDL;
   parameters->iterations = 1;
   strcpy(parameters->psInputFileName, "none");
   parameters->predefinedSubs = FALSE;
   parameters->minVertices = 1;
   parameters->maxVertices = 0; // i.e., infinity
   parameters->recursion = FALSE;
   parameters->variables = FALSE;
   parameters->relations = FALSE;
   parameters->incremental = FALSE;
   parameters->compress = FALSE;
   // GBAD
   parameters->mdl = FALSE;
   parameters->mdlThreshold = 0.0;
   parameters->prob = FALSE;
   parameters->mps = FALSE;
   parameters->maxAnomalousScore = MAX_DOUBLE;
   parameters->minAnomalousScore = 0.0;
   parameters->minOnly = FALSE;
   parameters->noAnomalyDetection = TRUE;
   // GBAD

   if (argc < 2)
   {
      fprintf(stderr, "input graph file name must be supplied\n");
      exit(1);
   }

   // process command-line options
   i = 1;
   while (i < (argc - 1))
   {
      if (strcmp(argv[i], "-beam") == 0) 
      {
         i++;
         sscanf(argv[i], "%lu", &ulongArg);
         if (ulongArg == 0) 
         {
            fprintf(stderr, "%s: beam must be greater than zero\n", argv[0]);
            exit(1);
         }
         parameters->beamWidth = ulongArg;
      }
      else if (strcmp(argv[i], "-compress") == 0)
      {
         parameters->compress = TRUE;
      }
      else if (strcmp(argv[i], "-eval") == 0) 
      {
         i++;
         sscanf(argv[i], "%lu", &ulongArg);
         if ((ulongArg < 1) || (ulongArg > 3)) 
         {
            fprintf(stderr, "%s: eval must be 1-3\n", argv[0]);
            exit(1);
         }
         parameters->evalMethod = ulongArg;
      } 
      else if (strcmp(argv[i], "-inc") == 0)
      {
         parameters->incremental = TRUE;
      }
      else if (strcmp(argv[i], "-iterations") == 0) 
      {
         i++;
         sscanf(argv[i], "%lu", &ulongArg);
         parameters->iterations = ulongArg;
      } 
      else if (strcmp(argv[i], "-limit") == 0) 
      {
         i++;
         sscanf(argv[i], "%lu", &ulongArg);
         if (ulongArg == 0) 
         {
            fprintf(stderr, "%s: limit must be greater than zero\n", argv[0]);
            exit(1);
         }
         parameters->limit = ulongArg;
         limitSet = TRUE;
      }
      else if (strcmp(argv[i], "-maxsize") == 0) 
      {
         i++;
         sscanf(argv[i], "%lu", &ulongArg);
         if (ulongArg == 0) 
         {
            fprintf(stderr, "%s: maxsize must be greater than zero\n", argv[0]);
            exit(1);
         }
         parameters->maxVertices = ulongArg;
      }
      else if (strcmp(argv[i], "-minsize") == 0) 
      {
         i++;
         sscanf(argv[i], "%lu", &ulongArg);
         if (ulongArg == 0) 
         {
            fprintf(stderr, "%s: minsize must be greater than zero\n", argv[0]);
            exit(1);
         }
         parameters->minVertices = ulongArg;
      }
      else if (strcmp(argv[i], "-nsubs") == 0) 
      {
         i++;
         sscanf(argv[i], "%lu", &ulongArg);
         if (ulongArg == 0) 
         {
            fprintf(stderr, "%s: nsubs must be greater than zero\n", argv[0]);
            exit(1);
         }
         parameters->numBestSubs = ulongArg;
      }
      else if (strcmp(argv[i], "-out") == 0) 
      {
         i++;
         strcpy(parameters->outFileName, argv[i]);
         parameters->outputToFile = TRUE;
      }
      else if (strcmp(argv[i], "-output") == 0) 
      {
         i++;
         sscanf(argv[i], "%lu", &ulongArg);
         if ((ulongArg < 1) || (ulongArg > 5)) 
         {
            fprintf(stderr, "%s: output must be 1-5\n", argv[0]);
            exit(1);
         }
         parameters->outputLevel = ulongArg;
      }
      else if (strcmp(argv[i], "-overlap") == 0) 
      {
         parameters->allowInstanceOverlap = TRUE;
      }
      else if (strcmp(argv[i], "-prune") == 0) 
      {
         parameters->prune = TRUE;
      }
      else if (strcmp(argv[i], "-ps") == 0) 
      {
         i++;
         strcpy(parameters->psInputFileName, argv[i]);
         parameters->predefinedSubs = TRUE;
      }
      else if (strcmp(argv[i], "-recursion") == 0) 
      {
         parameters->recursion = TRUE;
      }
      else if (strcmp(argv[i], "-relations") == 0) 
      {
         parameters->relations = TRUE;
         parameters->variables = TRUE; // relations must involve variables
      }
      else if (strcmp(argv[i], "-threshold") == 0) 
      {
         i++;
         sscanf(argv[i], "%lf", &doubleArg);
         if ((doubleArg < (double) 0.0) || (doubleArg > (double) 1.0))
         {
            fprintf(stderr, "%s: threshold must be 0.0-1.0\n", argv[0]);
            exit(1);
         }
         parameters->threshold = doubleArg;
      }
      else if (strcmp(argv[i], "-undirected") == 0) 
      {
         parameters->directed = FALSE;
      }
      else if (strcmp(argv[i], "-valuebased") == 0) 
      {
         parameters->valueBased = TRUE;
      }
      else if (strcmp(argv[i], "-variables") == 0) 
      {
         parameters->variables = TRUE;
      }
      //
      // GBAD
      //
      else if (strcmp(argv[i], "-mdl") == 0) 
      {
         i++;
         sscanf(argv[i], "%lf", &doubleArg);
         if ((doubleArg <= (double) 0.0) || (doubleArg >= (double) 1.0))
         {
            fprintf(stderr, "%s: Information Theoretic (MDL) threshold must be greater than 0.0 and less than 1.0\n", argv[0]);
            exit(1);
         }
         parameters->mdl = TRUE;
         parameters->mdlThreshold = doubleArg;
      }
      else if (strcmp(argv[i], "-prob") == 0) 
      {
         i++;
         sscanf(argv[i], "%lu", &ulongArg);
         if (ulongArg < 2)
         {
            fprintf(stderr, "%s: number of iterations should be greater than using the probabilistic anomaly detection method.\n", argv[0]);
            exit(1);
         }
         parameters->prob = TRUE;
         parameters->iterations = ulongArg;  // overrides -iterations specification
      }
      else if (strcmp(argv[i], "-mps") == 0) 
      {
         parameters->mps = TRUE;
      }
      else if (strcmp(argv[i], "-maxAnomalousScore") == 0) 
      {
         i++;
         sscanf(argv[i], "%lf", &doubleArg);
         if ((doubleArg <= (double) 0.0) || (doubleArg >= (double) MAX_DOUBLE))
         {
            fprintf(stderr, "%s: maximum anomalous score must be greater than 0.0 and less than %lf\n", argv[0],MAX_DOUBLE);
            exit(1);
         }
         parameters->maxAnomalousScore = doubleArg;
      }
      else if (strcmp(argv[i], "-minAnomalousScore") == 0) 
      {
         i++;
         sscanf(argv[i], "%lf", &doubleArg);
         if ((doubleArg < (double) 0.0) || (doubleArg >= (double) MAX_DOUBLE))
         {
            fprintf(stderr, "%s: minimum anomalous score must be greater than or equal to 0.0 and less than %lf\n", argv[0],MAX_DOUBLE);
            exit(1);
         }
         parameters->minAnomalousScore = doubleArg;
      }
      else if (strcmp(argv[i], "-minOnly") == 0) 
      {
         parameters->minOnly = TRUE;
      }
      //
      // GBAD
      //
      else 
      {
         fprintf(stderr, "%s: unknown option %s\n", argv[0], argv[i]);
         exit(1);
      }
      i++;
   }

   //
   // GBAD
   //
   if ((parameters->mdl) || (parameters->prob) || (parameters->mps)) 
      parameters->noAnomalyDetection = FALSE;
   //
   // GBAD
   //

   if (parameters->iterations == 0)
      parameters->iterations = MAX_UNSIGNED_LONG; // infinity

   // initialize log2Factorial[0..1]
   parameters->log2Factorial = (double *) malloc(2 * sizeof(double));
   if (parameters->log2Factorial == NULL)
      OutOfMemoryError("GetParameters:parameters->log2Factorial");
   parameters->log2FactorialSize = 2;
   parameters->log2Factorial[0] = 0; // lg(0!)
   parameters->log2Factorial[1] = 0; // lg(1!)

   // read graphs from input file
   strcpy(parameters->inputFileName, argv[argc - 1]);
   parameters->labelList = AllocateLabelList();
   parameters->posGraph = NULL;
   parameters->negGraph = NULL;
   parameters->numPosEgs = 0;
   parameters->numNegEgs = 0;
   parameters->posEgsVertexIndices = NULL;
   parameters->negEgsVertexIndices = NULL;

   if (parameters->incremental)
   {
      if (parameters->predefinedSubs)
      {
         fprintf(stderr, "Cannot process predefined examples incrementally");
         exit(1);
      }
  
      if (parameters->evalMethod == EVAL_MDL)
      {
         fprintf(stderr, "Incremental Subdue does not support EVAL_MDL, ");
         fprintf(stderr, "switching to EVAL_SIZE\n");
         parameters->evalMethod = EVAL_SIZE;
      }

      if ((parameters->evalMethod == EVAL_SIZE) && (parameters->compress))
      {
         fprintf(stderr, "Incremental Subdue does not support compression, ");
         fprintf(stderr, "with EVAL_SIZE, turning compression off\n");
         parameters->compress = FALSE;
      }

      if (parameters->iterations > 1)
      {
         fprintf(stderr,
            "Incremental Subdue only one iteration, setting to 1\n");
         parameters->iterations = 1;
      }
   }
   else 
   {
      ReadInputFile(parameters);
      if (parameters->evalMethod == EVAL_MDL)
      {
         parameters->posGraphDL = MDL(parameters->posGraph,
                                     parameters->labelList->numLabels, parameters);
         if (parameters->negGraph != NULL)
         {
            parameters->negGraphDL =
               MDL(parameters->negGraph, parameters->labelList->numLabels,
                   parameters);
         }
      }
   }

   // read predefined substructures
   parameters->numPreSubs = 0;
   if (parameters->predefinedSubs)
      ReadPredefinedSubsFile(parameters);

   parameters->incrementList = malloc(sizeof(IncrementList));
   parameters->incrementList->head = NULL;

   if (parameters->incremental)
   {
      parameters->vertexList = malloc(sizeof(InstanceVertexList));
      parameters->vertexList->avlTreeList = malloc(sizeof(AvlTreeList));
      parameters->vertexList->avlTreeList->head = NULL;
   }

   // create output file, if given
   if (parameters->outputToFile) 
   {
      outputFile = fopen(parameters->outFileName, "w");
      if (outputFile == NULL) 
      {
         printf("ERROR: unable to write to output file %s\n",
                parameters->outFileName);
         exit(1);
      }
      fclose(outputFile);
   }  

   return parameters;
}


//---------------------------------------------------------------------------
// NAME: PostProcessParameters
//
// INPUTS: (Parameters *parameters)
//
// RETURN: (void)
//
// PURPOSE: Initialize parts of the parameters structure.  This must be called
//          after graph data is obtained.
//
// NOTE:    This code was separated from the other parametric processing
//          because of the need to handle some parameters differentely when
//          the incremental version of SUBDUE is executed.
//---------------------------------------------------------------------------
    
Parameters *PostProcessParameters(Parameters *parameters)
{
   Increment *increment = NULL;

   if (parameters->incremental)
      increment = GetCurrentIncrement(parameters);

   // Code from this point until end of function was moved from GetParameters
   if (parameters->numPosEgs == 0)
   {
      fprintf(stderr, "ERROR: no positive graphs defined\n");
      exit(1);
   }

   // Check bounds on discovered substructures' number of vertices
   if (parameters->maxVertices == 0)
      parameters->maxVertices = parameters->posGraph->numVertices;
   if (parameters->maxVertices < parameters->minVertices)
   {
      fprintf(stderr, "ERROR: minsize exceeds maxsize\n");
      exit(1);
   }

   // Set limit accordingly
   if (parameters->limit == 0)
   {
      if (parameters->incremental)
         parameters->limit = increment->numPosEdges / 2;
      else 
         parameters->limit = parameters->posGraph->numEdges / 2;
   }

   return parameters;
}


//---------------------------------------------------------------------------
// NAME: PrintParameters
//
// INPUTS: (Parameters *parameters)
//
// RETURN: (void)
//
// PURPOSE: Print selected parameters.
//---------------------------------------------------------------------------

void PrintParameters(Parameters *parameters)
{
   printf("Parameters:\n");
   printf("  Input file..................... %s\n",parameters->inputFileName);
   printf("  Predefined substructure file... %s\n",parameters->psInputFileName);
   printf("  Output file.................... %s\n",parameters->outFileName);
   printf("  Beam width..................... %lu\n",parameters->beamWidth);
   printf("  Compress....................... ");
   PrintBoolean(parameters->compress);
   printf("  Evaluation method.............. ");
   switch(parameters->evalMethod) 
   {
      case 1: printf("MDL\n"); break;
      case 2: printf("size\n"); break;
      case 3: printf("setcover\n"); break;
   }
   //
   // GBAD
   //
   if (parameters->mdl) {
      printf("  Anomaly Detection method....... Information Theoretic\n");
      printf("  Information Theoretic threshold %lf\n", parameters->mdlThreshold);
   }
   if (parameters->prob) {
      printf("  Anomaly Detection method....... Probabilistic\n");
   }
   if (parameters->mps)
      printf("  Anomaly Detection method....... Maximum Partial\n");
   if (parameters->noAnomalyDetection) 
      printf("  Anomaly Detection method....... NONE\n");
   if (!parameters->noAnomalyDetection) {
      if (parameters->maxAnomalousScore) {
         if (parameters->maxAnomalousScore == MAX_DOUBLE)
            printf("  Max Anomalous Score............ MAX\n");
         else
            printf("  Max Anomalous Score............ %lf\n",parameters->maxAnomalousScore);
      }
      if (parameters->minAnomalousScore) 
         printf("  Min Anomalous Score............ %lf\n",parameters->minAnomalousScore);
   }
   //
   // GBAD
   //
   printf("  'e' edges directed............. ");
   PrintBoolean(parameters->directed);
   printf("  Incremental.................... ");
   PrintBoolean(parameters->incremental);
   printf("  Iterations..................... ");
   if (parameters->iterations == 0)
      printf("infinite\n");
   else 
      printf("%lu\n", parameters->iterations);
   printf("  Limit.......................... %lu\n", parameters->limit);
   printf("  Minimum size of substructures.. %lu\n", parameters->minVertices);
   printf("  Maximum size of substructures.. %lu\n", parameters->maxVertices);
   printf("  Number of best substructures... %lu\n", parameters->numBestSubs);
   printf("  Output level................... %lu\n", parameters->outputLevel);
   printf("  Allow overlapping instances.... ");
   PrintBoolean(parameters->allowInstanceOverlap);
   printf("  Prune.......................... ");
   PrintBoolean(parameters->prune);
   printf("  Threshold...................... %lf\n", parameters->threshold);
   printf("  Value-based queue.............. ");
   PrintBoolean(parameters->valueBased);
   printf("  Recursion...................... ");
   PrintBoolean(parameters->recursion);
   printf("\n");

   printf("Read %lu total positive graphs\n", parameters->numPosEgs);
   if (parameters->numNegEgs > 0) 
      printf("Read %lu total negative graphs\n", parameters->numNegEgs);
   if (parameters->numPreSubs > 0)
      printf("Read %lu predefined substructures\n", parameters->numPreSubs);
   printf("\n");
}


//---------------------------------------------------------------------------
// NAME: FreeParameters
//
// INPUTS: (Parameters *parameters)
//
// RETURN: (void)
//
// PURPOSE: Free memory allocated for parameters.  Note that the
//          predefined substructures are de-allocated as soon as they are
//          processed, and not here.
//---------------------------------------------------------------------------

void FreeParameters(Parameters *parameters)
{
   FreeGraph(parameters->posGraph);
   FreeGraph(parameters->negGraph);
   FreeLabelList(parameters->labelList);
   free(parameters->posEgsVertexIndices);
   free(parameters->negEgsVertexIndices);
   free(parameters->log2Factorial);
   free(parameters);
}
