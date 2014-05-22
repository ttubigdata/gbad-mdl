//******************************************************************************
// discover.c
//
// Main discovery functions.
//
// Date      Name       Description
// ========  =========  ========================================================
// 08/12/09  Eberle     Initial version, taken from SUBDUE 5.2.1
//
//******************************************************************************

#include "gbad.h"


//******************************************************************************
// NAME: DiscoverSubs
//
// INPUTS: (Parameters *parameters)
//
// RETURN: (SubList) - list of best discovered substructures
//
// PURPOSE: Discover the best substructures in the graphs according to
// the given parameters.  Note that we do not allow a single-vertex
// substructure of the form "SUB_#" on to the discovery list to avoid
// continually replacing "SUB_<n>" with "SUB_<n+1>".
//******************************************************************************

SubList *DiscoverSubs(Parameters *parameters, ULONG currentIteration)
{
   SubList *parentSubList;
   SubList *childSubList;
   SubList *extendedSubList;
   SubList *discoveredSubList;
   SubListNode *parentSubListNode;
   SubListNode *extendedSubListNode;
   Substructure *parentSub;
   Substructure *extendedSub;

   //
   // get initial one-vertex substructures
   //
   parentSubList = GetInitialSubs(parameters);

   // parameters used
   ULONG limit          = parameters->limit;
   ULONG numBestSubs    = parameters->numBestSubs;
   ULONG beamWidth      = parameters->beamWidth;
   BOOLEAN valueBased   = parameters->valueBased;
   LabelList *labelList = parameters->labelList;
   BOOLEAN prune        = parameters->prune;
   ULONG maxVertices    = parameters->maxVertices;
   ULONG minVertices    = parameters->minVertices;
   ULONG outputLevel    = parameters->outputLevel;
   ULONG evalMethod     = parameters->evalMethod;

   discoveredSubList = AllocateSubList();
   while ((limit > 0) && (parentSubList->head != NULL)) 
   {
      parentSubListNode = parentSubList->head;
      childSubList = AllocateSubList();
      // extend each substructure in parent list
      while (parentSubListNode != NULL)
      {
         parentSub = parentSubListNode->sub;
         parentSubListNode->sub = NULL;
         if (outputLevel > 4) 
         {
            parameters->outputLevel = 1; // turn off instance printing
            printf("\nConsidering ");
            PrintSub(parentSub, parameters);
            printf("\n");
            parameters->outputLevel = outputLevel;
         }
         if ((((parentSub->numInstances > 1) && (evalMethod != EVAL_SETCOVER) &&
               (parameters->noAnomalyDetection)) ||
              ((parentSub->numInstances > 0) && 
               (!parameters->noAnomalyDetection))) && 
             (limit > 0))
         {
            limit--;
            if (outputLevel > 3)
               printf("%lu substructures left to be considered\n", limit);
            fflush(stdout);
            extendedSubList = ExtendSub(parentSub, parameters);
            //
            // If this is the first iteration, call SetExampleNumber
            // so that the edges in each of the possible instances
            // has the associated original example number.  This will make
            // it easier for tracking the anomalies later.
            //
            if (parameters->currentIteration == 1)
               SetExampleNumber(extendedSubList,parameters);
	    //
            extendedSubListNode = extendedSubList->head;
            while (extendedSubListNode != NULL) 
            {
               extendedSub = extendedSubListNode->sub;
               extendedSubListNode->sub = NULL;
               if (extendedSub->definition->numVertices <= maxVertices) 
               {
                  // evaluate each extension and add to child list
                  EvaluateSub(extendedSub, parameters);
                  if (prune && (extendedSub->value < parentSub->value)) 
                  {
                     FreeSub(extendedSub);
                  } 
                  else 
                  {
                     //
                     // Need to look at all extensions, so
                     // ignore the beam width (i.e., max number of
                     // substructures being kept on the list).
                     //
                     if ((parameters->prob) && (parameters->currentIteration > 1))
                        SubListInsert(extendedSub, childSubList, 0,
                                      TRUE, labelList);
                     else
                        SubListInsert(extendedSub, childSubList, beamWidth,
                                      valueBased, labelList);
                  }
               } 
               else 
               {
                  FreeSub(extendedSub);
               }
               extendedSubListNode = extendedSubListNode->next;
            }
            FreeSubList(extendedSubList);
         }
         // add parent substructure to final discovered list
         if (parentSub->definition->numVertices >= minVertices) 
         {
            if ((! SinglePreviousSub(parentSub, parameters)) || (parameters->prob))
            {
               if (outputLevel > 3)
                  PrintNewBestSub(parentSub, discoveredSubList, parameters);
               if (parameters->prob)
                  SubListInsert(parentSub, discoveredSubList, 0, FALSE, 
                                labelList);
               else
                  SubListInsert(parentSub, discoveredSubList, numBestSubs, FALSE,
                                labelList);
            }
         } 
         else 
         {
            FreeSub (parentSub);
         }
         parentSubListNode = parentSubListNode->next;
      }
      FreeSubList(parentSubList);
      parentSubList = childSubList;
      //
      // GBAD-P:  This allows us to create only single extensions after the
      //          first iteration (and the normative pattern has been found)
      //
      if ((parameters->prob) && (currentIteration > 1))
         break;
   }

   if ((limit > 0) && (outputLevel > 2))
      printf ("\nSubstructure queue empty.\n");

   // try to insert any remaining subs in parent list on to discovered list
   parentSubListNode = parentSubList->head;
   while (parentSubListNode != NULL) 
   {
      parentSub = parentSubListNode->sub;
      parentSubListNode->sub = NULL;
      if (parentSub->definition->numVertices >= minVertices) 
      {
         if ((! SinglePreviousSub(parentSub, parameters)) || (parameters->prob))
	 {
            if (outputLevel > 3)
               PrintNewBestSub(parentSub, discoveredSubList, parameters);
            if (parameters->prob)
               SubListInsert(parentSub, discoveredSubList, 0, FALSE, labelList);
            else
               SubListInsert(parentSub, discoveredSubList, numBestSubs, FALSE,
                             labelList);
         }
      } 
      else 
      {
         FreeSub(parentSub);
      }
      parentSubListNode = parentSubListNode->next;
   }
   FreeSubList(parentSubList);
   return discoveredSubList;
}


//******************************************************************************
// NAME: GetInitialSubs
//
// INPUTS: (Parameters *parameters)
//
// RETURN: (SubList *)
//
// PURPOSE: Return a list of substructures, one for each unique vertex
// label in the positive graph that has at least two instances.
//******************************************************************************

SubList *GetInitialSubs(Parameters *parameters)
{
   SubList *initialSubs;
   ULONG i, j;
   ULONG vertexLabelIndex;
   ULONG numInitialSubs;
   Graph *g;
   Substructure *sub;
   Instance *instance;

   // parameters used
   Graph *posGraph      = parameters->posGraph;
   LabelList *labelList = parameters->labelList;
   ULONG outputLevel    = parameters->outputLevel;
   ULONG startVertexIndex;

   startVertexIndex = 0;

   // reset labels' used flag
   for (i = 0; i < labelList->numLabels; i++)
      labelList->labels[i].used = FALSE;
  
   numInitialSubs = 0;
   initialSubs = AllocateSubList();
   for (i = startVertexIndex; i < posGraph->numVertices; i++)
   {
      vertexLabelIndex = posGraph->vertices[i].label;
      if (labelList->labels[vertexLabelIndex].used == FALSE) 
      {
         labelList->labels[vertexLabelIndex].used = TRUE;

         // create one-vertex substructure definition
         g = AllocateGraph(1, 0);
         g->vertices[0].label = vertexLabelIndex;
         g->vertices[0].numEdges = 0;
         g->vertices[0].edges = NULL;
         // allocate substructure
         sub = AllocateSub();
         sub->definition = g;
         sub->instances = AllocateInstanceList();
         // collect instances in positive graph
         j = posGraph->numVertices;
         do 
         {
            j--;
            if (posGraph->vertices[j].label == vertexLabelIndex) 
            {
               // ***** do inexact label matches here? (instance->minMatchCost
               // ***** too)
               instance = AllocateInstance(1, 0);
               instance->vertices[0] = j;
               instance->mapping[0].v1 = 0;
               instance->mapping[0].v2 = j;
               instance->minMatchCost = 0.0;
               InstanceListInsert(instance, sub->instances, FALSE);
               sub->numInstances++;
            }
         } while (j > i);

         //
         // GBAD-P:  Only keep substructures if more than one positive
         //          instance, or if GBAD-P approach is used and we
         //          are past the first iteration after the normative
         //          pattern is discovered.
         //
	 //          Since for both GBAD-MDL and GBAD-MPS we need to keep
	 //          a list of all substructures, we are removing this
	 //          constraint when either of these algorithms are
	 //          specified.
	 //
	 if ((sub->numInstances > 1) ||
             ((parameters->prob) && (parameters->currentIteration > 2)) ||
	     (parameters->mdl) || (parameters->mps))
         {
            EvaluateSub(sub, parameters);
            // add to initialSubs
            SubListInsert(sub, initialSubs, 0, FALSE, labelList);
            numInitialSubs++;
         } 
         else 
         { // prune single-instance substructure
            FreeSub(sub);
         }
      }
   }
   if (outputLevel > 1)
      printf("%lu initial substructures\n", numInitialSubs);

   return initialSubs;
}


//******************************************************************************
// NAME: SinglePreviousSub
//
// INPUTS: (Substructure *sub) - substructure to check
//         (Parameters *parameters)
//
// RETURN: (BOOLEAN)
//
// PURPOSE: Returns TRUE if the given substructure is a single-vertex
// substructure and the vertex refers to a previously-discovered
// substructure, i.e., the vertex label is of the form "SUB_#".  This
// is used to prevent repeatedly compressing the graph by replacing a
// "SUB_<n>" vertex by a "SUB_<n+1>" vertex.
//******************************************************************************

BOOLEAN SinglePreviousSub(Substructure *sub, Parameters *parameters)
{
   BOOLEAN match;
   // parameters used
   LabelList *labelList = parameters->labelList;

   match = FALSE;
   if ((sub->definition->numVertices == 1) &&  // single-vertex sub?
       (SubLabelNumber (sub->definition->vertices[0].label, labelList) > 0))
       // valid substructure label
      match = TRUE;

   return match;
}
