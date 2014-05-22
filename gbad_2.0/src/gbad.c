//******************************************************************************
// gbad.c
//
// GBAD functions for determining the anomalies.
//
// Date      Name       Description
// ========  =========  ========================================================
// 08/12/09  Eberle     Initial version.
//
//******************************************************************************

#include "gbad.h"
#include "time.h"
#include <unistd.h>
#include <sys/times.h>

//******************************************************************************
// NAME: FlagAnomalousVerticesAndEdges
//
// INPUTS: (InstanceList *instanceList) - list of anomalous instances
//         (Graph *graph) - graph definition
//         (SubList *subList) - list of substructures
//         (Parameters *parameters)
//
// RETURN: (void)
//
// PURPOSE: Compare the two instances (their graph structures) and find all of
// the anomalous vertices and edges.
//******************************************************************************
void FlagAnomalousVerticesAndEdges(InstanceList *instanceList, Graph *graph,
                                   Substructure *sub, Parameters *parameters)
{
   Graph *instanceGraph;
   InstanceListNode *instanceListNode;
   Instance *instance;
   Edge *edge1;
   Edge *edge2;
   Vertex *vertex1;
   Vertex *vertex2;
   ULONG v, v2, e, e1, e2;
   ULONG numAnomalousVertices;
   ULONG *anomalousVertices;
   ULONG numAnomalousEdges;
   ULONG *anomalousEdges;
   VertexMap *mapping;
   ULONG *sortedMapping;
   ULONG maxVertices;
   ULONG i;
   double matchCost;
   ULONG edge1_v1;
   ULONG edge1_v2;
   ULONG edge2_v1;
   ULONG edge2_v2;
   BOOLEAN found;

   instanceListNode = instanceList->head;
   while (instanceListNode != NULL)
   {
      instance = instanceListNode->instance;
      instanceGraph = InstanceToGraph(instance, graph);
      //
      // NOTE: This assumes that we can not always guarantee which
      //       graph is the bigger one.
      //
      if (sub->definition->numVertices < instanceGraph->numVertices)
      {
         maxVertices = instanceGraph->numVertices;
         mapping = (VertexMap *) malloc(sizeof(VertexMap) * maxVertices);
         matchCost = InexactGraphMatch(instanceGraph, sub->definition,
                                       parameters->labelList, MAX_DOUBLE,
                                       mapping);
      }
      else
      {
         maxVertices = sub->definition->numVertices;
         mapping = (VertexMap *) malloc(sizeof(VertexMap) * maxVertices);
         matchCost = InexactGraphMatch(sub->definition, instanceGraph,
                                       parameters->labelList, MAX_DOUBLE,
                                       mapping);
      }
      sortedMapping = (ULONG *) malloc(sizeof(ULONG) * maxVertices);
      if (sortedMapping == NULL)
         OutOfMemoryError("FlagAnomalousVerticesAndEdges:  sortedMapping");
      for (i = 0; i < maxVertices; i++)
         sortedMapping[mapping[i].v1] = mapping[i].v2;

      anomalousVertices = (ULONG *) malloc(sizeof(ULONG) *
                          instanceGraph->numVertices);
      numAnomalousVertices = 0;
      //
      // First, compare vertices
      //
      for (v2 = 0; v2 < instanceGraph->numVertices; v2++)
      {
         vertex1 = & sub->definition->vertices[v2];
         vertex2 = & instanceGraph->vertices[sortedMapping[v2]];
         if (vertex1->label != vertex2->label)
         {
            anomalousVertices[numAnomalousVertices] =
                          instance->vertices[sortedMapping[v2]];
            numAnomalousVertices++;
         }
      }
      anomalousEdges = (ULONG *) malloc(sizeof(ULONG) * instanceGraph->numEdges);
      numAnomalousEdges = 0;
      //
      // Second, compare the edges
      //
      for (e2 = 0; e2 < instanceGraph->numEdges; e2++)
      {
         edge2 = & instanceGraph->edges[e2];
         edge2_v1 = edge2->vertex1;
         edge2_v2 = edge2->vertex2;

         found = FALSE;
         for (e1 = 0; e1 < sub->definition->numEdges; e1++)
         {
            edge1 = & sub->definition->edges[e1];
            edge1_v1 = edge1->vertex1;
            edge1_v2 = edge1->vertex2;
            if ((edge2_v1 == sortedMapping[edge1_v1]) &&
                (edge2_v2 == sortedMapping[edge1_v2]) &&
                (edge1->label == edge2->label)) {
               found = TRUE;
            }
         }
         if (!found) {
            anomalousEdges[numAnomalousEdges] = instance->edges[e2];
            numAnomalousEdges++;
         }
      }
      //
      // Now, save the actual anomalies
      //
      for (v=0;v<numAnomalousVertices;v++)
      {
         instance->anomalousVertices[instance->numAnomalousVertices] =
            anomalousVertices[v];
         instance->numAnomalousVertices++;
      }
      for (e=0;e<numAnomalousEdges;e++)
      {
         instance->anomalousEdges[instance->numAnomalousEdges] =
	    anomalousEdges[e];
         instance->numAnomalousEdges++;
      }
      //
      free(anomalousVertices);
      instanceListNode = instanceListNode->next;
      free(sortedMapping);
      free(mapping);
      FreeGraph(instanceGraph);
   }
}


//******************************************************************************
// NAME: PrintProbabilisticAnomalies (GBAD-P)
//
// INPUTS: (InstanceList *instanceList) - list of anomalous instances
//         (Parameters *parameters)
//
// RETURN: (void)
//
// PURPOSE: This routine prints the anomalous instance(s) found in the list of 
//          instances.
//
// NOTE: This routine assumes that AddAnomalousInstancesToBestSubstructure has 
//       been called first.
//******************************************************************************
void PrintProbabilisticAnomalies(InstanceList *instanceList,
                                 Parameters *parameters)
{
   InstanceListNode *firstInstanceListNode = NULL;
   ULONG count = 0;
   ULONG i;

   Graph *posGraph = parameters->posGraph;

   //
   // Print all instances with the lowest probability that are equal to or
   // below the user-specified threshold.
   //
   firstInstanceListNode = instanceList->head;
   if (firstInstanceListNode != NULL)
      printf("Anomalous Instance(s): ");
   else
   {
      printf("Anomalous Instance(s): NONE\n");
      return;
   }
   while (firstInstanceListNode != NULL)
   {
      if ((firstInstanceListNode->instance->probAnomalousValue <=
           parameters->maxAnomalousScore) &&
          (firstInstanceListNode->instance->probAnomalousValue >=
           parameters->minAnomalousScore))
      {
         printf("\n");
         //
         // For the probabilistic method, if the normative pattern has been
         // compressed (i.e. this is after the first iteration), any vertices
         // (except SUB_) and edges that are part of this anomalous instance
         // would be anomalous
         //
         if (parameters->currentIteration > 1)
         {
            //
            // This for loop handles the marking of the specific anomalous
            // vertices
            //
            for (i=0;i<firstInstanceListNode->instance->numVertices;i++)
            {
               if ((parameters->labelList->labels[posGraph->vertices[firstInstanceListNode->instance->vertices[i]].label].labelType == STRING_LABEL) &&
	           (strncmp(parameters->labelList->labels[posGraph->vertices[firstInstanceListNode->instance->vertices[i]].label].labelValue.stringLabel,
		            "SUB_",4)))
               {
                  firstInstanceListNode->instance->anomalousVertices[firstInstanceListNode->instance->numAnomalousVertices] = 
                     firstInstanceListNode->instance->vertices[i];
                  firstInstanceListNode->instance->numAnomalousVertices++;
               }
            }
            //
            // This for loop handles the marking of the specific anomalous
            // edges
            //
            for (i=0;i<firstInstanceListNode->instance->numEdges;i++)
            {
               firstInstanceListNode->instance->anomalousEdges[firstInstanceListNode->instance->numAnomalousEdges] = 
                  firstInstanceListNode->instance->edges[i];
               firstInstanceListNode->instance->numAnomalousEdges++;
            }
         }
         if (count == 0)
	    printf("\n");
         count++;
         ULONG posEgNo;
         ULONG numPosEgs = parameters->numPosEgs;
         ULONG *posEgsVertexIndices = parameters->posEgsVertexIndices;
         posEgNo = InstanceExampleNumber(firstInstanceListNode->instance,
                                         posEgsVertexIndices, numPosEgs);
         printf(" from positive example %lu:\n", posEgNo);
         PrintAnomalousInstance(firstInstanceListNode->instance, posGraph, 
	                        parameters);
         printf("    (probabilistic anomalous value = %f )\n",
                firstInstanceListNode->instance->probAnomalousValue);
      }
      firstInstanceListNode = firstInstanceListNode->next;
   }
   if (count == 0)
      printf("NONE\n");
}


//******************************************************************************
// NAME: StoreAnomalousEdge
//
// INPUTS: (Edge *overlapEdges) - edge array where edge is stored
//         (ULONG edgeIndex) - index into edge array where edge is stored
//         (ULONG v1) - vertex1 of edge
//         (ULONG v2) - vertex2 of edge
//         (ULONG label) - edge label index
//         (BOOLEAN directed) - edge directedness
//         (BOOLEAN spansIncrement) - edge crossing increment boundary
//         (BOOLEAN anomalou) - edge is anomalous
//
// RETURN: (void)
//
// PURPOSE: Procedure to store an edge in given edge array.
//
// NOTE: Currently, this procedure is only called from the routines in 
//       compress.c.
//******************************************************************************

void StoreAnomalousEdge(Edge *overlapEdges, ULONG edgeIndex,
                        ULONG v1, ULONG v2, ULONG label, BOOLEAN directed,
                        BOOLEAN spansIncrement, BOOLEAN anomalous,
			ULONG sourceVertex1, ULONG sourceVertex2,
			ULONG sourceExample)
{
   overlapEdges[edgeIndex].vertex1 = v1;
   overlapEdges[edgeIndex].vertex2 = v2;
   overlapEdges[edgeIndex].label = label;
   overlapEdges[edgeIndex].directed = directed;
   overlapEdges[edgeIndex].used = FALSE;
   overlapEdges[edgeIndex].spansIncrement = spansIncrement;
   overlapEdges[edgeIndex].anomalous = anomalous;
   overlapEdges[edgeIndex].sourceVertex1 = sourceVertex1;
   overlapEdges[edgeIndex].sourceVertex2 = sourceVertex2;
   overlapEdges[edgeIndex].sourceExample = sourceExample;
}


//******************************************************************************
// NAME: PrintAnomalousVertex
//
// INPUTS: (Graph *graph) - graph containing vertex
//         (ULONG vertexIndex) - index of vertex to print
//         (LabelList *labelList) - labels in graph
//         (Instance *instance) - instance to be marked
//         (Parameters *parameters)
//
// RETURN: (void)
//
// PURPOSE: Print a vertex.
//******************************************************************************

void PrintAnomalousVertex(Graph *graph, ULONG vertexIndex, LabelList *labelList,
                          Instance *instance, Parameters *parameters)
{
   printf("v %lu ", vertexIndex + 1);
   PrintLabel(graph->vertices[vertexIndex].label, labelList);
   //
   // I have hacked this in the sense that it is setting the anomalous flag
   // in a print routine... thus, if this routine is not called, the vertex
   // will not get its flag set correctly....
   //
   if (instance != NULL)
   {
      ULONG i;
      for (i=0;i<instance->numAnomalousVertices;i++)
      {
         if (instance->anomalousVertices[i] == vertexIndex)
         {
            // Don't print the word "anomaly" next to every vertex when using 
	    // the MPS algorithm because the entire structure is anomalous
            if (!parameters->mps)
               printf(" <-- anomaly");
            graph->vertices[vertexIndex].anomalous = TRUE;
            printf(" (original vertex: %lu , in original example %lu)",
	           graph->vertices[vertexIndex].sourceVertex,
	           graph->vertices[vertexIndex].sourceExample);
            break;
         }
      }
   }
   printf("\n");
}


//******************************************************************************
// NAME: PrintAnomalousEdge
//
// INPUTS: (Graph *graph) - graph containing edge
//         (ULONG edgeIndex) - index of edge to print
//         (LabelList *labelList) - labels in graph
//         (Instance *instance) - instance to be marked
//         (Parameters *parameters)
//
// RETURN: (void)
//
// PURPOSE: Print an edge.
//******************************************************************************

void PrintAnomalousEdge(Graph *graph, ULONG edgeIndex, LabelList *labelList,
                        Instance *instance, Parameters *parameters)
{
   Edge *edge = & graph->edges[edgeIndex];

   if (edge->directed)
      printf("d");
   else
      printf("u");
   printf(" %lu %lu ", edge->vertex1 + 1, edge->vertex2 + 1);
   PrintLabel(edge->label, labelList);
   //
   // I have hacked this in the sense that it is setting the anomalous flag
   // in a print routine... thus, if this routine is not called, the vertex
   // will not get its flag set correctly....
   //
   if (instance != NULL)
   {
      ULONG i;
      for (i=0;i<instance->numAnomalousEdges;i++)
      {
         if (instance->anomalousEdges[i] == edgeIndex)
         {
            // Don't print the word "anomaly" next to every vertex when using 
	    // the MPS algorithm because the entire structure is anomalous
            if (!parameters->mps)
               printf(" <-- anomaly");
            graph->edges[edgeIndex].anomalous = TRUE;
            printf(" (original edge vertices: %lu -- %lu, in original example %lu)",
	           graph->edges[edgeIndex].sourceVertex1,
	           graph->edges[edgeIndex].sourceVertex2,
	           graph->edges[edgeIndex].sourceExample);
            break;
         }
      }
   }
   printf("\n");
}


//******************************************************************************
// NAME: PrintAnomalousInstance
//
// INPUTS: (Instance *instance) - instance to print
//         (Graph *graph) - graph containing instance
//         (LabelList *labelList) - labels from graph
//
// RETURN: (void)
//
// PURPOSE: Print given instance.
//******************************************************************************

void PrintAnomalousInstance(Instance *instance, Graph *graph,
                            Parameters *parameters)
{
   ULONG i;
   LabelList *labelList = parameters->labelList;

   if (instance != NULL)
   {
      for (i = 0; i < instance->numVertices; i++)
      {
         printf("    ");
         PrintAnomalousVertex(graph, instance->vertices[i], labelList, 
	                      instance, parameters);
      }
      for (i = 0; i < instance->numEdges; i++)
      {
         printf("    ");
         PrintAnomalousEdge(graph, instance->edges[i], labelList, 
	                    instance, parameters);
      }
   }
}


//******************************************************************************
// NAME: SetExampleNumber
//
// INPUTS: (SubList *subList) - substructure list containing substructures
//                              of positive instances
//         (Parameters *parameters)
//
// RETURN: (void)
//
// PURPOSE: Set the sourceExample field in every edge.
//
// NOTE:  It is intended that this utility would be called only on the first
//        iteration.  Otherwise, the example number being set will not be
//        useful.
//
// NOTE:  Currently, this procedure is only called from discover.c.
//******************************************************************************

void SetExampleNumber(SubList *subList, Parameters *parameters)
{
   ULONG i;
   ULONG posEgNo;
   InstanceListNode *instanceListNode;
   Instance *instance;
   SubListNode *subListNode = NULL;
   Substructure *sub = NULL;

   // parameters used
   Graph *graph = parameters->posGraph;
   ULONG numPosEgs = parameters->numPosEgs;
   ULONG *posEgsVertexIndices = parameters->posEgsVertexIndices;

   if (subList != NULL)
   {
      subListNode = subList->head;
      while (subListNode != NULL)
      {
         sub = subListNode->sub;
         if (sub->instances != NULL)
         {
            instanceListNode = sub->instances->head;
            while (instanceListNode != NULL)
            {
               if (numPosEgs > 1)
               {
                  instance = instanceListNode->instance;
                  posEgNo = InstanceExampleNumber(instance,
                                                  posEgsVertexIndices, 
						  numPosEgs);
                  for (i = 0; i < instance->numEdges; i++)
                     graph->edges[instance->edges[i]].sourceExample = posEgNo;
                  for (i = 0; i < instance->numVertices; i++)
                     graph->vertices[instance->vertices[i]].sourceExample = posEgNo;
               }
               instanceListNode = instanceListNode->next;
            }
         }
         subListNode = subListNode->next;
      }
   }
}


//******************************************************************************
// NAME: RemoveSimilarSubstructures
//
// INPUTS: (SubList *subList) - list of substructures
//         (Parameters *parameters)
//
// RETURN: (void) - BUT list of substructure is modified
//
// PURPOSE: The purpose of this routine is to remove those substructures
// that are too similar to the best substructure in the list, based upon the
// user specified "similarity" parameter.  If a substructure in the list is
// greater than the user-specified similarity percentage, the 
// substructure is removed from the list (actually, just the pointers are 
// redirected).
//
// The primary purpose of this function is to allow the user to process a list
// of substructures that are not so similar in structure.
//
// NOTE:  The default parameters->similarity value is 100% (or 1.0).  This
//        means that we are willing to accept substructures that are UP TO
//        100% similar to the best substructure.  Conversely, a value of 0.0
//        means that the substructure has to be completely different from
//        the best substructure (i.e., the lower the value, the more
//        dissimilar are two substructures).  So, if the value is 0.75,
//        that means that we are willing to accept a substructure that is
//        up to 75% similar to the best substructure.  Meaning, that if
//        a substructure is more than 75% similar to the best substructure,
//        it will be thrown out.
//
// NOTE:  Currently, this routine is only called from main.c.
//******************************************************************************
void RemoveSimilarSubstructures(SubList *subList, Parameters *parameters)
{
   SubListNode *subIndex = NULL;
   SubListNode *subIndexPrevious = NULL;
   SubListNode *subIndexNext = NULL;
   Graph *bestGraph = NULL;
   double matchCost;
   double matchChange = 0.0;
   double totalSize = 0;
   //double numVerticesEdges = 0;

   //
   // First, calculate how much change (matchCost) is needed in order for
   // the percentage of requested change to be acceptable.
   //
   subIndex = subList->head;
   bestGraph = subIndex->sub->definition;
   // two changes per vertex/edge - label and existence...
   totalSize = 2 * (bestGraph->numVertices + bestGraph->numEdges);
   matchChange = (1.0 - parameters->similarity) * (double) totalSize;

   //
   // Then, see which substructures have enough change as compared to the
   // best substructure.
   //
   // First substructure is the best, so all comparisons will be to it...
   subIndexPrevious = subIndex;
   subIndex = subIndex->next;
   while (subIndex != NULL)
   {
      GraphMatch(bestGraph,subIndex->sub->definition,parameters->labelList,
                 MAX_DOUBLE, & matchCost, NULL);
      //
      // Keep the substructure if it has enough of a difference...
      // (or the user has specified to keep everything, with a value of 1.0)
      //
      if (((matchCost >= matchChange) && (parameters->similarity != 0.0)) || 
          (parameters->similarity == 1.0) ||
          ((matchCost == 0.0) && (parameters->similarity == 0.0)))
      {
         subIndexPrevious = subIndex;
      }
      //
      // Else, this substructure is too close in similarity to the best
      // substructure, so remove it from the list of substructures
      //
      else 
      {
         subIndexNext = subIndex->next;
         subIndexPrevious->next = subIndexNext;
      }

      subIndex = subIndex->next;
   }
}

//******************************************************************************
// NAME: AddAnomalousInstancesToBestSubstructure (GBAD-P)
//
// INPUTS: (SubList *subList) - list of substructures
//         (Parameters *parameters)
//
// RETURN: (SubList) - modified list of substructures
//
// PURPOSE: The purpose of this routine is to add the anomalous instances to
// the best substructure, so that we can later "loop through" the possible
// anomalies and print the relevant one(s).
//
//******************************************************************************
SubList* AddAnomalousInstancesToBestSubstructure(SubList *subList, 
                                                 Parameters *parameters)
{
   SubListNode *subListNode = NULL;
   SubList *bestSubList = NULL;
   Substructure *bestSub = NULL;
   Instance *instance = NULL;
   InstanceList *instanceList = NULL;
   InstanceListNode *instanceListNode = NULL;
   InstanceList *anomInstanceList = NULL;
   SubList *newSubList = NULL;
   newSubList = AllocateSubList();
   double currentAnomalousValue = 0.0;
   double minAnomalousValue = 1.0;

   ULONG i;

   char subLabelString[TOKEN_LEN];
   sprintf(subLabelString, "%s_%lu", SUB_LABEL_STRING, (parameters->currentIteration-1));

   //
   // First, find all instances that include edges from the previously
   // compressed substructure, and use that to determine the number of
   // extensions from the best substructure.
   //
   subListNode = subList->head;
   bestSub = CopySub(subListNode->sub);
   bestSub->numInstances = 0;
   while (subListNode != NULL)
   {
      for (i=0;i<subListNode->sub->definition->numVertices;i++)
      {
         if ((parameters->labelList->labels[subListNode->sub->definition->vertices[i].label].labelType == STRING_LABEL) &&
             (parameters->labelList->labels[subListNode->sub->definition->vertices[i].label].labelValue.stringLabel != NULL) &&
             (subLabelString != NULL) &&
             (subListNode->sub->definition->numEdges > 0) &&
	     (!strncmp(parameters->labelList->labels[subListNode->sub->definition->vertices[i].label].labelValue.stringLabel,
	               subLabelString,5)))
         {
            bestSub->numInstances = bestSub->numInstances + subListNode->sub->numInstances;
	    break;
         }
      }
      subListNode = subListNode->next;
   }
   if (bestSub != NULL) 
   {
      printf("Normative Pattern:\n");
      PrintSub(bestSub,parameters);
      printf("\n");
   } else
      return NULL;

   //
   // Second, loop through all substructures, and find the lowest score
   // (which indicates the best anomalous value).
   //
   subListNode = subList->head;
   while (subListNode != NULL)
   {
      for (i=0;i<subListNode->sub->definition->numVertices;i++)
      {
         if ((parameters->labelList->labels[subListNode->sub->definition->vertices[i].label].labelType == STRING_LABEL) &&
             (parameters->labelList->labels[subListNode->sub->definition->vertices[i].label].labelValue.stringLabel != NULL) &&
             (subLabelString != NULL) &&
             (subListNode->sub->definition->numEdges > 0) &&
	     (!strncmp(parameters->labelList->labels[subListNode->sub->definition->vertices[i].label].labelValue.stringLabel,
	               subLabelString,5)))
         {
            currentAnomalousValue = (double)subListNode->sub->numInstances /
                                    (double)parameters->numPreviousInstances;
	    if (currentAnomalousValue < minAnomalousValue)
	    {
	       minAnomalousValue = currentAnomalousValue;
	       break;
            }
         }
      }
      subListNode = subListNode->next;
   }

   //
   // Then, loop through all of the substructures, and find the anomalous instances
   // that match the lowest score.
   //
   // NOTE:  Only look at substructures that contain "SUB_" in its substructure.
   //        (stored in variable subLabelString)
   //
   subListNode = subList->head;
   anomInstanceList = AllocateInstanceList();
   while (subListNode != NULL)
   {
      for (i=0;i<subListNode->sub->definition->numVertices;i++)
      {
         if ((parameters->labelList->labels[subListNode->sub->definition->vertices[i].label].labelType == STRING_LABEL) &&
             (parameters->labelList->labels[subListNode->sub->definition->vertices[i].label].labelValue.stringLabel != NULL) &&
             (subLabelString != NULL) &&
             (subListNode->sub->definition->numEdges > 0) &&
	     (!strncmp(parameters->labelList->labels[subListNode->sub->definition->vertices[i].label].labelValue.stringLabel,
	               subLabelString,5)))
         {
            currentAnomalousValue = (double)subListNode->sub->numInstances /
	                            (double)parameters->numPreviousInstances;
            // The reason for the following unusual comparison is that there is
	    // not a good way to compare two floating type variables.  I am
	    // using this for now, BUT, if the precision needs to go out beyond
	    // 6 digits to the right, this will not work.
            if (fabs(currentAnomalousValue - minAnomalousValue) < 0.000001)
            {
	       //
	       // Set propabilistic value for all of the instances to the
	       // same anomalous value
	       //
               instanceList = subListNode->sub->instances;
               instanceListNode = instanceList->head;
               while (instanceListNode != NULL)
               {
                  instance = instanceListNode->instance;
                  instance->probAnomalousValue = (double)currentAnomalousValue;
                  InstanceListInsert(instance, anomInstanceList, FALSE);
                  instanceListNode = instanceListNode->next;
               }
	       break;
	    }
         }
      }
      subListNode = subListNode->next;
   }

   //
   // Finally, add all of the anomalous instances to the substructure that is
   // providing the normative pattern (i.e., set its number of instances to all
   // possible best anomalies).
   //
   if (anomInstanceList != NULL)
   {
      bestSub->instances = AllocateInstanceList();
      bestSub->numInstances = 0;
      instanceListNode = anomInstanceList->head;
      while (instanceListNode != NULL)
      {
         if (instanceListNode->instance != NULL)
         {
            instance = instanceListNode->instance;
            InstanceListInsert(instance, bestSub->instances, FALSE);
            bestSub->numInstances++;
            instanceListNode = instanceListNode->next;
         }
      }
   } else
      return NULL;

   FreeInstanceList(anomInstanceList);

   //
   // Create new best substructure list
   //
   bestSubList = AllocateSubList();

   // Put best substructure and instances on list (will be the ONLY one)
   SubListInsert(bestSub, bestSubList, 0, FALSE, parameters->labelList);

   return bestSubList;
}

//******************************************************************************
// NAME: FindAnomalousInstances
//
// INPUTS: (Substructure *sub) - substructure to search for
//         (Graph *g2) - graph to search in
//         (Parameters *parameters)
//
// RETURN: (InstanceList *) - list of instances of sub in g2, may be empty
//
// PURPOSE: Searches for subgraphs of g2 that match sub1 and returns the
// list of such subgraphs as instances in g2.  Returns empty list if
// no matches exist.  This procedure mimics the DiscoverSubs loop by
// repeatedly expanding instances of subgraphs of sub in g2 until
// matches are found.  The procedure is optimized toward g1 being a
// small graph and g2 being a large graph.
//
// Note: This procedure is equivalent to the NP-Hard subgraph
// isomorphism problem, and therefore can be quite slow for some
// graphs.
//******************************************************************************

InstanceList *FindAnomalousInstances(Substructure *sub, Graph *g2, 
                                     Parameters *parameters)
{
   ULONG v;
   ULONG v1;
   ULONG e1;
   Vertex *vertex1;
   Edge *edge1;
   InstanceList *instanceList;
   BOOLEAN *reached;
   BOOLEAN noMatches;
   BOOLEAN found;
   Graph *g1 = sub->definition;

   reached = (BOOLEAN *) malloc(sizeof(BOOLEAN) * g1->numVertices);
   if (reached == NULL)
      OutOfMemoryError("FindAnomalousInstances:reached");
   for (v1 = 0; v1 < g1->numVertices; v1++)
      reached[v1] = FALSE;

   //
   // In order to make sure we don't miss the anomalous instance without
   // having to examine every possible combination, we need to specify
   // an initial set of single vertices that will guarantee proper
   // coverage
   //
   ULONG i, j;
   ULONG vertexLabelIndex;
   ULONG numInitialVerticesToConsider;
   Instance *instance;

   instanceList = AllocateInstanceList();
   numInitialVerticesToConsider = ((g1->numVertices + g1->numEdges) * parameters->mdlThreshold) + 1;

   for (j = 0; j < numInitialVerticesToConsider; j++)
   {
      for (i = 0; i < g2->numVertices; i++)
      {
         vertexLabelIndex = g2->vertices[i].label;
         if (g1->vertices[j].label == vertexLabelIndex)
         {
            instance = AllocateInstance(1, 0);
            instance->vertices[0] = i;
            instance->minMatchCost = 0.0;
            InstanceListInsert(instance, instanceList, FALSE);
            reached[j] = TRUE;
         }
      }
   }

   v1 = 0; // first vertex in g1
   vertex1 = & g1->vertices[v1];

   noMatches = FALSE;
   if (instanceList->head == NULL) // no matches to vertex1 found in g2
      noMatches = TRUE;
   while ((vertex1 != NULL) && (! noMatches))
   {
      vertex1->used = TRUE;
      // extend by each unmarked edge involving vertex v1
      for (e1 = 0; ((e1 < vertex1->numEdges) && (! noMatches)); e1++)
      {
         edge1 = & g1->edges[g1->vertices[v1].edges[e1]];
         if (! edge1->used)
         {
            reached[edge1->vertex1] = TRUE;
            reached[edge1->vertex2] = TRUE;
            instanceList = ExtendAnomalousInstancesByEdge(instanceList, sub, 
                                                       g1, edge1, g2, 
                                                       parameters->mdlThreshold,
                                                       parameters);
            if (instanceList->head == NULL)
               noMatches = TRUE;
            edge1->used = TRUE;
         }
      }
      // find next un-used, reached vertex
      vertex1 = NULL;
      found = FALSE;
      for (v = 0; ((v < g1->numVertices) && (! found)); v++)
      {
         if ((! g1->vertices[v].used) && (reached[v]))
         {
            v1 = v;
            vertex1 = & g1->vertices[v1];
            found = TRUE;
         }
      }
   }
   free(reached);
   return instanceList;
}


//******************************************************************************
// NAME: ExtendAnomalousInstancesByEdge
//
// INPUTS: (InstanceList *instanceList) - instances to extend by one edge
//         (Substructure *sub) - substructure (best)
//         (Graph *g1) - graph whose instances we are looking for
//         (Edge *edge1) - edge in g1 by which to extend each instance
//         (Graph *g2) - graph containing instances
//         (double threshold) - user-specified threshold of change
//         (Parameters parameters)
//
// RETURN: (InstanceList *) - new instance list with extended instances
//
// PURPOSE: Attempts to extend each instance in instanceList by an
// edge from graph g2 that matches the attributes of the given edge in
// graph g1.  Returns a new (possibly empty) instance list containing
// the extended instances.  The given instance list is de-allocated.
//******************************************************************************

InstanceList *ExtendAnomalousInstancesByEdge(InstanceList *instanceList,
                                             Substructure *sub, Graph *g1, 
                                             Edge *edge1, Graph *g2,
                                             double threshold,
                                             Parameters *parameters)
{
   InstanceList *newInstanceList;
   InstanceListNode *instanceListNode;
   Instance *instance;
   Instance *newInstance = NULL;
   ULONG v2;
   ULONG e2;
   Edge *edge2;
   Vertex *vertex2;
   Graph *instanceGraph;
   double matchCost;
   double thresholdLimit = threshold * (g1->numVertices + g1->numEdges);
   ULONG deltaSize;
   double tempThreshold;
   BOOLEAN overlaps;

   newInstanceList = AllocateInstanceList();
   // extend each instance
   instanceListNode = instanceList->head;
   while (instanceListNode != NULL)
   {
      instance = instanceListNode->instance;
      //
      // See if the instance overlaps with any of the
      // best substructure instances; if so, we can skip doing any 
      // extensions because it will never be an anomalous instance.
      //
      overlaps = InstanceListOverlap(instance,sub->instances);
      if (!overlaps)
      {
         instanceGraph = InstanceToGraph(instance, g2);
         deltaSize = (g1->numVertices + g1->numEdges) -
                     (instanceGraph->numVertices + instanceGraph->numEdges) + 1;
         tempThreshold = thresholdLimit + (double)deltaSize;
         MarkInstanceEdges(instance, g2, TRUE);
         // consider extending from each vertex in instance
         for (v2 = 0; v2 < instance->numVertices; v2++)
         {
            vertex2 = & g2->vertices[instance->vertices[v2]];
            for (e2 = 0; e2 < vertex2->numEdges; e2++)
            {
               edge2 = & g2->edges[vertex2->edges[e2]];
               if ((! edge2->used) &&
                   ((EdgesMatch(g1, edge1, g2, edge2, parameters)) ||
                    (GraphMatch(g1, instanceGraph, parameters->labelList,
                                tempThreshold, & matchCost, NULL))))
               {
                  if (matchCost < instance->minMatchCost)
                     instance->minMatchCost = matchCost;
                  newInstance = CreateExtendedInstance(instance,
                                                       instance->vertices[v2],
                                                       vertex2->edges[e2], g2,
                                                       FALSE);
                  InstanceListInsert(newInstance, newInstanceList, TRUE);
               }
            }
         }
         MarkInstanceEdges(instance, g2, FALSE);
         FreeGraph(instanceGraph);
      }
      instanceListNode = instanceListNode->next;
   }

   FreeInstanceList(instanceList);
   return newInstanceList;
}


//******************************************************************************
// NAME: ScoreAndPrintAnomalousInstances
//
// INPUTS: (InstanceList *instanceList) - list of potential anomalous instances
//         (Substructure *sub) - normative pattern substructure
//         (Parameters *parameters)
//
// RETURN: void
//
// PURPOSE:  This routine takes the list of potential anomalous instances
//           (i.e., instances that did not match the normative pattern within
//           a user-defined threshold), determines their anomalous score, and
//           prints out the most anomalous instance(s).
//******************************************************************************
void ScoreAndPrintAnomalousInstances(InstanceList *instanceList, 
                                     Substructure *sub,
                                     Parameters *parameters)
{
   double matchCost;
   double anomalousValue = 0.0;
   ULONG minimumValue = MAX_UNSIGNED_LONG;
   Instance *otherInstance = NULL;
   Instance *instance = NULL;
   InstanceListNode *otherInstanceListNode = NULL;
   InstanceListNode *instanceListNode = NULL;
   InstanceList *anomalousInstanceList = NULL;
   InstanceList *reducedInstanceList = NULL;
   Graph *instanceGraph;
   Graph *otherInstanceGraph;
   LabelList *labelList = parameters->labelList;
   Graph *posGraph = parameters->posGraph;

   //
   // First, remove the instances that match the normative pattern (and would
   // not be anomalous) OR they overlap with instances of the normative
   // pattern.
   //
   reducedInstanceList = AllocateInstanceList();
   instanceListNode = instanceList->head;
   while (instanceListNode != NULL)
   {
      if (instanceListNode->instance != NULL)
      {
         instance = instanceListNode->instance;
         //
         // Then see if their structures are the same
         //
         if (instance->numVertices == sub->definition->numVertices)
         {
            instanceGraph = InstanceToGraph(instance, posGraph);
            GraphMatch(sub->definition,instanceGraph,labelList, MAX_DOUBLE, 
                       & matchCost, NULL);
            if (matchCost > 0.0)
            {
               instance->minMatchCost = matchCost;
               InstanceListInsert(instance, reducedInstanceList, FALSE);
            }
            FreeGraph(instanceGraph);
         }
         else
            InstanceListInsert(instance, reducedInstanceList, FALSE);
      }
      instanceListNode = instanceListNode->next;
   }
   //
   //
   // Second, determine how many instances have the same substructure
   // pattern (for frequency value)
   //
   instanceListNode = reducedInstanceList->head;
   while (instanceListNode != NULL)
   {
      instance = instanceListNode->instance;
      if ((instance != NULL) && (!instance->matched))
      {
         instanceGraph = InstanceToGraph(instance, posGraph);
         instance->frequency = 0;
         otherInstanceListNode = reducedInstanceList->head;
         while (otherInstanceListNode != NULL)
         {
            otherInstance = otherInstanceListNode->instance;
            if (otherInstance != NULL)
            {
               if ((instance->numVertices == otherInstance->numVertices) &&
                   (instance->numEdges == otherInstance->numEdges))
               {
                  otherInstanceGraph = InstanceToGraph(otherInstance, posGraph);
                  GraphMatch(instanceGraph, otherInstanceGraph, labelList,
                             MAX_DOUBLE, & matchCost, NULL);
                  if (matchCost == 0.0)
                  {
                     instance->frequency++;
                     otherInstance->matched = TRUE;
                  }
                  FreeGraph(otherInstanceGraph);
               }
            }
            otherInstanceListNode = otherInstanceListNode->next;
         }
         instance->matched = TRUE;
         //
         // Now, to speed things up, go back and set the frequency to the same
         // value for all of the matching instances, so that they do not have to
         // be scored again
         //
         otherInstanceListNode = reducedInstanceList->head;
         while (otherInstanceListNode != NULL)
         {
            otherInstance = otherInstanceListNode->instance;
            if ((otherInstance != NULL) && (otherInstance->matched))
               otherInstance->frequency = instance->frequency;
            otherInstanceListNode = otherInstanceListNode->next;
         }
         //
         FreeGraph(instanceGraph);
      }
      instanceListNode = instanceListNode->next;
   }
   //
   // Third, calculate anomalous score for all of the potential anomalous
   // instances.
   //
   instanceListNode = reducedInstanceList->head;
   while (instanceListNode != NULL)
   {
      if (instanceListNode->instance != NULL)
      {
         instance = instanceListNode->instance;
         //
         // Calculate anomalous value of this instance
         //
         anomalousValue = (double) instance->minMatchCost *
                          (double) instance->frequency;
         instance->infoAnomalousValue = anomalousValue;
         if (instance->infoAnomalousValue <= (double) minimumValue)
         {
            minimumValue = instance->infoAnomalousValue;
         }
      }
      instanceListNode = instanceListNode->next;
   }
   //
   // Fourth, only save those instances that match the minimumValue and the
   // user-specified criteria
   //
   instanceListNode = reducedInstanceList->head;
   anomalousInstanceList = AllocateInstanceList();
   while (instanceListNode != NULL)
   {
      if (instanceListNode->instance != NULL)
      {
         instance = instanceListNode->instance;
         if ((instance->infoAnomalousValue == minimumValue) &&
             (instanceListNode->instance->infoAnomalousValue <= parameters->maxAnomalousScore) &&
             (instanceListNode->instance->infoAnomalousValue >= parameters->minAnomalousScore))
            InstanceListInsert(instance, anomalousInstanceList, FALSE);
      }
      instanceListNode = instanceListNode->next;
   }
   //
   // Finally, loop through the final list of anomalous instances, flag the
   // anomalous vertices and edges, and print the anomalies.
   //
   if (anomalousInstanceList != NULL)
   {
      FlagAnomalousVerticesAndEdges(anomalousInstanceList, posGraph,
                                    sub, parameters);
      //
      instanceListNode = anomalousInstanceList->head;
      if (instanceListNode != NULL)
      {
         printf("Anomalous Instance(s):\n");
         while (instanceListNode != NULL)
         {
            printf("\n");
            ULONG posEgNo;
            ULONG numPosEgs = parameters->numPosEgs;
            ULONG *posEgsVertexIndices = parameters->posEgsVertexIndices;
            posEgNo = InstanceExampleNumber(instanceListNode->instance,
                                            posEgsVertexIndices,
                                            numPosEgs);
            printf(" from example %lu:\n", posEgNo);
            PrintAnomalousInstance(instanceListNode->instance, 
                                   posGraph, parameters);
            printf("    (information_theoretic anomalous value = %f )\n",
                   instanceListNode->instance->infoAnomalousValue);
            instanceListNode = instanceListNode->next;
         }
      } else {
         printf("Anomalous Instances:  NONE.\n");
      }
   } else {
      printf("Anomalous Instances:  NONE.\n");
   }

   return;
}


//******************************************************************************
// NAME: GBAD_MDL
//
// INPUTS: (SubList *subList) - list of top-N substructures
//         (Parameters *parameters)
//
// RETURN: void
//
// PURPOSE:  This routine takes the list of top-K substructures, and applies
//           the GBAD-MDL algorithm.  The purpose of this algorithm is to
//           search for all instances of the best substructure (or best N
//           substructures) in the graph that are within a user-specified
//           threshold of "change acceptance".  Then, the instance(s) that
//           are closest in matching to the best substructure WITHOUT matching
//           exactly, is output as anomalous.
//******************************************************************************
void GBAD_MDL(SubList *subList, Parameters *parameters)
{
   InstanceList *potentialAnomalousInstanceList;
   SubListNode *subListNode = NULL;
   SubListNode *bestSubListNode = NULL;
   Substructure *bestSub = NULL;
   ULONG i;
   ULONG bestNumVertices;
   Graph *posGraph = parameters->posGraph;

   //
   // First, find the normative pattern the user has requested
   // (if they have not requested one, then the default
   // value will be 1 - i.e., the best substructure)
   //
   if (parameters->norm > 1)
   {
      subListNode = subList->head;
      bestSubListNode = subListNode;
      for (i=1; i<parameters->norm; i++)
      {
         subListNode = subListNode->next;
         if (subListNode == NULL)
         {
            printf("WARNING:  The user specified a normative pattern that does");
	    printf(" not exist (%lu) - the best substructure will be chosen.\n",
	           parameters->norm);
            bestSubListNode = subList->head;
	    i = parameters->norm;
         } else
            bestSubListNode = subListNode;
      }
      bestSub = bestSubListNode->sub;
      bestNumVertices = bestSubListNode->sub->instances->head->instance->numVertices;
   }
   else
   {
      bestSub = subList->head->sub;
      bestNumVertices = subList->head->sub->instances->head->instance->numVertices;
   }

   printf("Normative Pattern (%lu):\n",parameters->norm);
   PrintSub(bestSub,parameters);
   printf("\n");

   //
   // Set maxVertices and minVertices to the size of the normative
   // pattern because for GBAD-MDL there is no point in examining
   // graphs of other sizes
   //
   parameters->maxVertices = bestNumVertices;
   parameters->minVertices = bestNumVertices;

   //
   // Then, search for potentially anomalous instances in the graph
   //
   potentialAnomalousInstanceList =
      FindAnomalousInstances(bestSub, posGraph, parameters);

   //
   // Then, score and print the anomalies (if any)
   //
   if (potentialAnomalousInstanceList != NULL)
   {
      ScoreAndPrintAnomalousInstances(potentialAnomalousInstanceList,
                                      bestSub,
                                      parameters);
   }

   return;
}


//******************************************************************************
// NAME: GBAD_MPS
//
// INPUTS: (SubList *subList) - list of top-N substructures
//         (Parameters *parameters)
//
// RETURN: void
//
// PURPOSE:  This routine takes the list of top-K substructures, and applies
//           the GBAD-MPS algorithm.  The purpose of this algorithm is to
//           search for all "ancestral" instances of the best substructure (or 
//           best N substructures) in the graph that are within a user-specified
//           threshold of "change acceptance".  Then, the instance(s) that
//           are closest in matching to the best substructure WITHOUT matching
//           exactly, is output as anomalous.
//******************************************************************************
void GBAD_MPS(SubList *subList, Parameters *parameters)
{
   InstanceList *potentialAnomalousInstanceList;
   SubListNode *subListNode = NULL;
   SubListNode *bestSubListNode = NULL;
   Substructure *bestSub = NULL;
   ULONG i;
   ULONG bestNumVertices;
   Graph *posGraph = parameters->posGraph;

   //
   // First, find the normative pattern the user has requested
   // (if they have not requested one, then the default
   // value will be 1 - i.e., the best substructure)
   //
   if (parameters->norm > 1)
   {
      subListNode = subList->head;
      bestSubListNode = subListNode;
      for (i=1; i<parameters->norm; i++)
      {
         subListNode = subListNode->next;
         if (subListNode == NULL)
         {
            printf("WARNING:  The user specified a normative pattern that does");
	    printf(" not exist (%lu) - the best substructure will be chosen.\n",
	           parameters->norm);
            bestSubListNode = subList->head;
	    i = parameters->norm;
         } else
            bestSubListNode = subListNode;
      }
      bestSub = bestSubListNode->sub;
      bestNumVertices = bestSubListNode->sub->instances->head->instance->numVertices;
   }
   else
   {
      bestSub = subList->head->sub;
      bestNumVertices = subList->head->sub->instances->head->instance->numVertices;
   }

   printf("Normative Pattern (%lu):\n",parameters->norm);
   PrintSub(bestSub,parameters);
   printf("\n");

   //
   // Set maxVertices to the number of vertices in the normative pattern,
   // because there is no point in examining graphs that are larger, and set
   // minVertices to the minimum number of vertices it could possibly be.
   //
   parameters->maxVertices = bestNumVertices;
   parameters->minVertices = bestNumVertices - 
                             (bestNumVertices * parameters->mpsThreshold);

   //
   // Then, search for potentially anomalous instances in the graph
   //
   potentialAnomalousInstanceList =
      FindPotentialAnomalousAncestors(bestSub, posGraph, parameters);

   //
   // Then, score and print the anomalies (if any)
   //
   if (potentialAnomalousInstanceList != NULL)
   {
      ScoreAndPrintAnomalousAncestors(potentialAnomalousInstanceList,
                                      bestSub,
                                      parameters);
   }

   return;
}


//******************************************************************************
// NAME: FindAnomalousAncestors (GBAD-MPS)
//
// INPUTS: (Substructure *sub) - substructure to search for
//         (Graph *g2) - graph to search in
//         (Parameters *parameters)
//
// RETURN: (InstanceList *) - list of ancestral instances of sub in g2, 
//                            may be empty
//
// PURPOSE: Searches for subgraphs of g2 that match sub1 and returns the
// list of such subgraphs as instances in g2.  Returns empty list if
// no matches exist.  This procedure mimics the DiscoverSubs loop by
// repeatedly expanding instances of subgraphs of sub in g2 until
// matches are found.  The procedure is optimized toward g1 being a
// small graph and g2 being a large graph.
//
// Note: This procedure is equivalent to the NP-Hard subgraph isomorphism 
//       problem, and therefore can be quite slow for some graphs.
//
// Note: This procedure assumes that an anomalous instance can not be just a 
//       single vertex.
//
// Note: This procedure is for the GBAD-MPS algorithm, and as such, assumes that
//       the parameters->mpsThreshold is set.
//
//******************************************************************************

InstanceList *FindAnomalousAncestors(Substructure *sub, Graph *g2, 
                                     Parameters *parameters)
{
   ULONG v;
   ULONG v1;
   ULONG e1;
   ULONG firstVertex = 0;
   Vertex *vertex1;
   Edge *edge1;
   InstanceList *instanceList;
   InstanceList *newInstanceList;
   InstanceListNode *instanceListNode;
   BOOLEAN *reached;
   BOOLEAN noMatches;
   BOOLEAN found;
   BOOLEAN overlaps;
   Graph *g1 = sub->definition;
   ULONG i, j;
   ULONG vertexLabelIndex;
   ULONG numInitialVerticesToConsider;
   Instance *instance;

   reached = (BOOLEAN *) malloc(sizeof(BOOLEAN) * g1->numVertices);
   if (reached == NULL)
      OutOfMemoryError("FindAnomalousAncestors:reached");
   for (v1 = 0; v1 < g1->numVertices; v1++)
      reached[v1] = FALSE;
   //
   // In order to make sure we don't miss the anomalous instance without
   // having to examing every possible combination, we need to specify
   // an initial set of single vertices that will guarantee proper
   // coverage
   //
   instanceList = AllocateInstanceList();

   numInitialVerticesToConsider = ((g1->numVertices + g1->numEdges) * 
                                   parameters->mpsThreshold) + 1;

   for (j = 0; j < numInitialVerticesToConsider; j++)
   {
      for (i = 0; i < g2->numVertices; i++)
      {
         vertexLabelIndex = g2->vertices[i].label;
         if (g1->vertices[j].label == vertexLabelIndex)
         {
            instance = AllocateInstance(1, 0);
            instance->vertices[0] = i;
            instance->minMatchCost = 0.0;
            overlaps = InstanceListOverlap(instance,sub->instances);
            if (!overlaps)
            {
               InstanceListInsert(instance, instanceList, FALSE);
               reached[j] = TRUE;
               if (firstVertex == 0)
                  firstVertex = j;
            }
         }
      }
   }

   newInstanceList = AllocateInstanceList();
   v1 = firstVertex; // first reached vertex in g1
   vertex1 = & g1->vertices[v1];

   noMatches = FALSE;
   if (instanceList->head == NULL) // no matches to vertex1 found in g2
      noMatches = TRUE;
   while ((vertex1 != NULL) && (! noMatches))
   {
      vertex1->used = TRUE;
      // extend by each unmarked edge involving vertex v1
      for (e1 = 0; ((e1 < vertex1->numEdges) && (! noMatches)); e1++)
      {
         edge1 = & g1->edges[g1->vertices[v1].edges[e1]];
         if (! edge1->used)
         {
            reached[edge1->vertex1] = TRUE;
            reached[edge1->vertex2] = TRUE;
            instanceList = ExtendAnomalousInstancesByEdge(instanceList, sub, 
                                                       g1, edge1, g2, 
                                                       parameters->mpsThreshold,
                                                       parameters);
            if (instanceList->head == NULL)
               noMatches = TRUE;
            edge1->used = TRUE;
         }
      }
      // find next un-used, reached vertex
      vertex1 = NULL;
      found = FALSE;
      for (v = 0; ((v < g1->numVertices) && (! found)); v++)
      {
         if ((! g1->vertices[v].used) && (reached[v]))
         {
            v1 = v;
            vertex1 = & g1->vertices[v1];
            found = TRUE;
         }
      }
      //
      // Now add this instance list to what we already have
      //
      if (instanceList != NULL)
      {
         instanceListNode = instanceList->head;
         while (instanceListNode != NULL)
         {
            InstanceListInsert(instanceListNode->instance, newInstanceList, 
                               TRUE);
            instanceListNode = instanceListNode->next;
         }
      }
   }
   free(reached);

   return newInstanceList;
}


//******************************************************************************
// NAME: ScoreAndPrintAnomalousAncestors (GBAD-MPS)
//
// INPUTS: (InstanceList *instanceList - list of potential ancestor instances
//         (Subtructure *sub) - best substructure
//         (Parameters *parameters)
//
// RETURN: (void)
//
// PURPOSE:  This routine looks at all of the potential anomalous instances
// If any instance is a partial match (i.e. all of its vertices and edges are 
// included in the list of instances of the best substructure), and it does
// not match with any of the instances of the best substructure (such that
// it would eventually extend to the best substructure), then output the
// one with the lowest (cost of transformation * frequency).
//******************************************************************************

void ScoreAndPrintAnomalousAncestors(InstanceList *instanceList, 
                                     Substructure *sub, Parameters *parameters)
{
   double matchCost;
   double anomalousValue = 0.0;
   ULONG minimumValue = MAX_UNSIGNED_LONG;
   Instance *otherInstance = NULL;
   Instance *instance = NULL;
   InstanceListNode *otherInstanceListNode = NULL;
   InstanceListNode *instanceListNode = NULL;
   InstanceList *anomalousInstanceList = NULL;
   InstanceList *reducedInstanceList = NULL;
   Graph *instanceGraph;
   Graph *otherInstanceGraph;
   LabelList *labelList = parameters->labelList;
   Graph *posGraph = parameters->posGraph;
   BOOLEAN foundBest = FALSE;
   double bestMatchThreshold = 1.0;
   ULONG bestNumVertices = 0;
   double matchThreshold;

   //
   // First, remove the instances that match the normative pattern (and would
   // not be anomalous) OR they overlap with instances of the normative
   // pattern, and calculate the matching cost of those instances that are
   // potentially anomalous.
   //
   reducedInstanceList = AllocateInstanceList();
   instanceListNode = instanceList->head;
   if (instanceListNode != NULL)
      if (instanceListNode->instance != NULL)
         bestNumVertices = instanceListNode->instance->numVertices;

   while ((instanceListNode != NULL) && (!foundBest))
   {
      if (instanceListNode->instance != NULL)
      {
         instance = instanceListNode->instance;
         //
         // Calculate the match cost between this instance and the best
         // substructure
         //
         instanceGraph = InstanceToGraph(instance, posGraph);
         GraphMatch(sub->definition,instanceGraph,labelList, MAX_DOUBLE, 
                    & matchCost, NULL);
         matchThreshold = matchCost / 
                     (sub->definition->numVertices + sub->definition->numEdges);
         //
         // If the match cost is higher than the previous best match cost, AND
         // the structure is getting smaller, there can't be a better choice
         // for an anomalous instance
         //
         if ((matchThreshold > bestMatchThreshold) && 
             (instance->numVertices < bestNumVertices))
            foundBest = TRUE;

         //
         // If the match cost is less than or equal to the user-specified
         // threshold(s), save it on the list
         //
         if ((matchThreshold <= parameters->mpsThreshold) &&
             (matchThreshold != 0.0) &&
             (matchCost <= parameters->maxAnomalousScore))
         {
            instance->minMatchCost = matchCost;
            if (matchThreshold < bestMatchThreshold)
            {
               bestMatchThreshold = matchThreshold;
               bestNumVertices = instance->numVertices;
            }
            InstanceListInsert(instance, reducedInstanceList, FALSE);
         }
         FreeGraph(instanceGraph);
      }
      instanceListNode = instanceListNode->next;
   }
   //
   //
   // Second, determine how many instances have the same substructure
   // pattern (for frequency value)
   //
   instanceListNode = reducedInstanceList->head;
   while (instanceListNode != NULL)
   {
      instance = instanceListNode->instance;
      if ((instance != NULL) && (!instance->matched))
      {
         instanceGraph = InstanceToGraph(instance, posGraph);
         instance->frequency = 0;
         otherInstanceListNode = reducedInstanceList->head;
         while (otherInstanceListNode != NULL)
         {
            otherInstance = otherInstanceListNode->instance;
            if (otherInstance != NULL)
            {
               if ((instance->numVertices == otherInstance->numVertices) &&
                   (instance->numEdges == otherInstance->numEdges))
               {
                  otherInstanceGraph = InstanceToGraph(otherInstance, posGraph);
                  GraphMatch(instanceGraph, otherInstanceGraph, labelList,
                             MAX_DOUBLE, & matchCost, NULL);
                  if (matchCost == 0.0)
                  {
                     instance->frequency++;
                     otherInstance->matched = TRUE;
                  }
                  FreeGraph(otherInstanceGraph);
               }
            }
            otherInstanceListNode = otherInstanceListNode->next;
         }
         instance->matched = TRUE;
         //
         // Now, to speed things up, go back and set the frequency to the same
         // value for all of the matching instances, so that they do not have to
         // be scored again
         //
         otherInstanceListNode = reducedInstanceList->head;
         while (otherInstanceListNode != NULL)
         {
            otherInstance = otherInstanceListNode->instance;
            if ((otherInstance != NULL) && (otherInstance->matched))
               otherInstance->frequency = instance->frequency;
            otherInstanceListNode = otherInstanceListNode->next;
         }
         //
         FreeGraph(instanceGraph);
      }
      instanceListNode = instanceListNode->next;
   }
   //
   // Third, calculate anomalous score for all of the potential anomalous
   // instances.
   //
   instanceListNode = reducedInstanceList->head;
   while (instanceListNode != NULL)
   {
      if (instanceListNode->instance != NULL)
      {
         instance = instanceListNode->instance;
         //
         // Calculate anomalous value of this instance
         //
         anomalousValue = (double) instance->minMatchCost *
                          (double) instance->frequency;
         instance->infoAnomalousValue = anomalousValue;
         if (instance->infoAnomalousValue <= (double) minimumValue)
         {
            minimumValue = instance->infoAnomalousValue;
         }
      }
      instanceListNode = instanceListNode->next;
   }
   //
   // Fourth, only save those instances that match the minimumValue and the
   // user-specified criteria
   //
   instanceListNode = reducedInstanceList->head;
   anomalousInstanceList = AllocateInstanceList();
   while (instanceListNode != NULL)
   {
      if (instanceListNode->instance != NULL)
      {
         instance = instanceListNode->instance;
         if ((instance->infoAnomalousValue == minimumValue) &&
             (instanceListNode->instance->infoAnomalousValue <= parameters->maxAnomalousScore) &&
             (instanceListNode->instance->infoAnomalousValue >= parameters->minAnomalousScore))
            InstanceListInsert(instance, anomalousInstanceList, FALSE);
      }
      instanceListNode = instanceListNode->next;
   }
   //
   // Fifth, remove any instances that overlap with another instance (keeping
   // one).  This would occur when instances are extended from different 
   // points, but never match exactly with each other.  However, clearly, if
   // they overlap at any point, they would have been "merged" into one.  So,
   // only keep one of them, which is enough to show the analyst where the
   // anomaly resides.
   //
   InstanceList *finalInstanceList = AllocateInstanceList();
   BOOLEAN overlaps;

   instanceListNode = anomalousInstanceList->head;
   while (instanceListNode != NULL)
   {
      if (instanceListNode->instance != NULL)
      {
         instanceListNode->instance->matched = FALSE;
      }
      instanceListNode = instanceListNode->next;
   }

   instanceListNode = anomalousInstanceList->head;
   while (instanceListNode != NULL)
   {
      instance = instanceListNode->instance;
      if ((instance != NULL) && (!instance->matched))
      {
         // Set the instance flag and insert onto final list
         instance->matched = TRUE;
         InstanceListInsert(instance, finalInstanceList, FALSE);
         // Then see if any other instances overlap, so they can be thrown away
         otherInstanceListNode = anomalousInstanceList->head;
         while (otherInstanceListNode != NULL)
         {
            otherInstance = otherInstanceListNode->instance;
            if ((otherInstance != NULL) && (!otherInstance->matched))
            {
               overlaps = InstanceOverlap(instance,otherInstance);
               if (overlaps)
                  otherInstance->matched = TRUE;
            }
            otherInstanceListNode = otherInstanceListNode->next;
         }
      }
      instanceListNode = instanceListNode->next;
   }
   //
   // Finally, loop through the final list of anomalous instances, flag the
   // anomalous vertices and edges, and print the anomalies.
   //
   if (finalInstanceList != NULL)
   {
      FlagAnomalousVerticesAndEdges(finalInstanceList, posGraph,
                                    sub, parameters);
      //
      instanceListNode = finalInstanceList->head;
      if (instanceListNode != NULL)
      {
         printf("Anomalous Instance(s):\n");
         while (instanceListNode != NULL)
         {
            printf("\n");
            ULONG posEgNo;
            ULONG numPosEgs = parameters->numPosEgs;
            ULONG *posEgsVertexIndices = parameters->posEgsVertexIndices;
            posEgNo = InstanceExampleNumber(instanceListNode->instance,
                                            posEgsVertexIndices,
                                            numPosEgs);
            printf(" from example %lu:\n", posEgNo);
            PrintAnomalousInstance(instanceListNode->instance, 
                                   posGraph, parameters);
            printf("    (information_theoretic anomalous value = %f )\n",
                   instanceListNode->instance->infoAnomalousValue);
            instanceListNode = instanceListNode->next;
         }
      } else {
         printf("Anomalous Instances:  NONE.\n");
      }
   } else {
      printf("Anomalous Instances:  NONE.\n");
   }

   return;
}


//******************************************************************************
// NAME: GBAD_P
//
// INPUTS: (SubList *subList) - list of top-N substructures
//         (ULONG) - iteration
//         (Parameters *parameters) - system parameters
//
// RETURN: (Substructure *) - normative substructure
//
// PURPOSE:  This routine takes the list of top-K substructures, and applies
//           the GBAD-P algorithm.  The purpose of this algorithm is to
//           set the best substructure to the user-specified substructure when
//           it is the first iteration.  Then, on subsequent iterations, create
//           a list of extended instances (from the best substructure
//           instances), and print the instance(s) with the lowest probability.
//           Also, if it is the first iteration, this function returns the
//           best substructure (which the user may have chosen something
//           other than the default best one), otherwise, it returns null.
//******************************************************************************

Substructure *GBAD_P(SubList *subList, ULONG iteration, Parameters *parameters)
{
   ULONG saveEvalMethod;
   SubList *newSubList = NULL;
   SubListNode *normSubListNode = NULL;

   if (iteration > 1)
   {
      //
      // If a different evaluation metric was used to find the best
      // substructure, change the evaluation technique for finding the
      // anomalous instance(s).
      //
      saveEvalMethod = parameters->evalMethod;
      parameters->evalMethod = EVAL_MDL;
      newSubList = AddAnomalousInstancesToBestSubstructure(subList,
                                                           parameters);

      //
      // If there is not a new substructure list, that means no anomalies
      // can be found, so skip the rest of this logic
      //
      if (newSubList != NULL)
      {
         //
         // If the best substructure does not have any instances, then
         // there are no anomalies.
         //
         if (newSubList->head != NULL)
            PrintProbabilisticAnomalies(newSubList->head->sub->instances,parameters);
         else
            printf("Anomalous Instances:  NONE.\n");
      }
      else
         printf("Anomalous Instances:  NONE.\n"); 
      //
      // If a different evaluation metric was used to find the best
      // substructure, change the evaluation technique for finding the
      // anomalous instance(s)
      //
      parameters->evalMethod = saveEvalMethod;
      return NULL;
   }
   else
   {
      ULONG i;
      printf("Normative Pattern (%lu):\n",parameters->norm);
      normSubListNode = subList->head;
      for (i=1; i<parameters->norm; i++)
      {
         normSubListNode = normSubListNode->next;
         if (normSubListNode == NULL)
         {
            printf("WARNING:  The user specified a normative");
            printf(" pattern that does not exist (%lu) - ",
                   parameters->norm);
            printf("the best substructure will be chosen.\n");
            normSubListNode = subList->head;
            i = parameters->norm;
         }
      }
      PrintSub(normSubListNode->sub,parameters);
      printf("\n");
      return normSubListNode->sub;
   }
}


//******************************************************************************
// NAME: FindPotentialAnomalousAncestors
//
// INPUTS: (Graph *g1) - graph to search for
//         (Graph *g2) - graph to search in
//         (Parameters *parameters)
//
// RETURN: (InstanceList *) - list of instances of g1 in g2, may be empty
//
// PURPOSE: Searches for subgraphs of g2 that match g1 and returns the
// list of such subgraphs as instances in g2.  Returns empty list if
// no matches exist.  This procedure mimics the DiscoverSubs loop by
// repeatedly expanding instances of subgraphs of g1 in g2 until
// matches are found.  The procedure is optimized toward g1 being a
// small graph and g2 being a large graph.
//
// Note: This procedure is equivalent to the NP-Hard subgraph
// isomorphism problem, and therefore can be quite slow for some
// graphs.
//******************************************************************************

InstanceList *FindPotentialAnomalousAncestors(Substructure *sub, Graph *g2, 
                                              Parameters *parameters)
{
   ULONG v;
   ULONG v1;
   ULONG e1;
   Vertex *vertex1;
   Edge *edge1;
   InstanceList *instanceList = NULL;
   InstanceList *parentInstanceList = NULL;
   InstanceListNode *instanceListNode = NULL;
   BOOLEAN *reached;
   BOOLEAN noMatches;
   BOOLEAN found;
   Graph *g1 = sub->definition;
   ULONG numInitialVerticesToConsider;
   ULONG i, j, vertexLabelIndex;
   ULONG firstVertex = 0;
   Instance *instance = NULL;
   BOOLEAN overlaps;

   parentInstanceList = AllocateInstanceList();

   reached = (BOOLEAN *) malloc(sizeof(BOOLEAN) * g1->numVertices);
   if (reached == NULL)
      OutOfMemoryError("FindInstances:reached");
   for (v1 = 0; v1 < g1->numVertices; v1++)
      reached[v1] = FALSE;

   //
   // In order to make sure we don't miss the anomalous instance without
   // having to examing every possible combination, we need to specify
   // an initial set of single vertices that will guarantee proper
   // coverage
   //
   instanceList = AllocateInstanceList();

   numInitialVerticesToConsider = ((g1->numVertices + g1->numEdges) * 
                                   parameters->mpsThreshold) + 1;

   for (j = 0; j < numInitialVerticesToConsider; j++)
   {
      for (i = 0; i < g2->numVertices; i++)
      {
         vertexLabelIndex = g2->vertices[i].label;
         if (g1->vertices[j].label == vertexLabelIndex)
         {
            instance = AllocateInstance(1, 0);
            instance->vertices[0] = i;
            instance->minMatchCost = 0.0;
            overlaps = InstanceListOverlap(instance,sub->instances);
            if (!overlaps)
            {
               InstanceListInsert(instance, instanceList, FALSE);
               reached[j] = TRUE;
               if (firstVertex == 0)
                  firstVertex = j;
            }
         }
      }
   }

   v1 = firstVertex; // first reached vertex in g1
   vertex1 = & g1->vertices[v1];
   noMatches = FALSE;
   if (instanceList->head == NULL) // no matches to vertex1 found in g2
      noMatches = TRUE;
   while ((vertex1 != NULL) && (! noMatches))
   {
      vertex1->used = TRUE;
      // extend by each unmarked edge involving vertex v1
      for (e1 = 0; ((e1 < vertex1->numEdges) && (! noMatches)); e1++)
      {
         edge1 = & g1->edges[g1->vertices[v1].edges[e1]];
         if (! edge1->used)
         {
            reached[edge1->vertex1] = TRUE;
            reached[edge1->vertex2] = TRUE;
            instanceList =
               ExtendPotentialInstancesByEdge(instanceList, g1, edge1, g2, sub, parameters);
            if (instanceList->head == NULL)
               noMatches = TRUE;
            edge1->used = TRUE;
         }
      }

      // find next un-used, reached vertex
      vertex1 = NULL;
      found = FALSE;
      for (v = 0; ((v < g1->numVertices) && (! found)); v++)
      {
         if ((! g1->vertices[v].used) && (reached[v]))
         {
            v1 = v;
            vertex1 = & g1->vertices[v1];
            found = TRUE;
         }
      }
      //
      // If number of vertices at this iteration is one less than size of
      // normative pattern, save instances on parentInstanceList
      //
      if (instanceList != NULL)
      {
         if (instanceList->head != NULL)
         {
            instanceListNode = instanceList->head;
            if (instanceListNode->instance != NULL)
            {
               if (instanceListNode->instance->numVertices < sub->definition->numVertices)
               {
                  while (instanceListNode != NULL)
                  {
                     InstanceListInsert(instanceListNode->instance, parentInstanceList,
                                        TRUE);
                     instanceListNode = instanceListNode->next;
                  }
               }
            }
         }
      }
   }
   free(reached);

   // set used flags to FALSE
   for (v1 = 0; v1 < g1->numVertices; v1++)
      g1->vertices[v1].used = FALSE;
   for (e1 = 0; e1 < g1->numEdges; e1++)
      g1->edges[e1].used = FALSE;

   return parentInstanceList;
}


//******************************************************************************
// NAME: ExtendPotentialInstancesByEdge
//
// INPUTS: (InstanceList *instanceList) - instances to extend by one edge
//         (Graph *g1) - graph whose instances we are looking for
//         (Edge *edge1) - edge in g1 by which to extend each instance
//         (Graph *g2) - graph containing instances
//         (Parameters parameters)
//
// RETURN: (InstanceList *) - new instance list with extended instances
//
// PURPOSE: Attempts to extend each instance in instanceList by an
// edge from graph g2 that matches the attributes of the given edge in
// graph g1.  Returns a new (possibly empty) instance list containing
// the extended instances.  The given instance list is de-allocated.
//******************************************************************************

InstanceList *ExtendPotentialInstancesByEdge(InstanceList *instanceList,
                                    Graph *g1, Edge *edge1, Graph *g2,
                                    Substructure *sub,
                                    Parameters *parameters)
{
   InstanceList *newInstanceList;
   InstanceListNode *instanceListNode;
   Instance *instance;
   Instance *newInstance;
   ULONG v2;
   ULONG e2;
   Edge *edge2;
   Vertex *vertex2;
   BOOLEAN overlaps = FALSE;
   BOOLEAN noExtensions = TRUE;

   newInstanceList = AllocateInstanceList();
   // extend each instance
   instanceListNode = instanceList->head;
   while (instanceListNode != NULL)
   {
      instance = instanceListNode->instance;
      MarkInstanceEdges(instance, g2, TRUE);
      //
      // See if the instance overlaps with any of the
      // best substructure instances; if so, we can skip doing any 
      // extensions because it will never be an anomalous instance.
      //
      overlaps = InstanceListOverlap(instance,sub->instances);
      if (!overlaps)
      {
         // consider extending from each vertex in instance
         for (v2 = 0; v2 < instance->numVertices; v2++)
         {
            vertex2 = & g2->vertices[instance->vertices[v2]];
            for (e2 = 0; e2 < vertex2->numEdges; e2++)
            {
               edge2 = & g2->edges[vertex2->edges[e2]];
               if (! edge2->used)
               {
                  // add new instance to list
                  newInstance =
                     CreateExtendedInstance(instance, instance->vertices[v2],
                                            vertex2->edges[e2], g2, FALSE);
                  noExtensions = FALSE;
                  InstanceListInsert(newInstance, newInstanceList, TRUE);
                  //
                  // To cut down on the number of possible "combinations",
                  // one an instance has been extended, don't extend any
                  // more - if it needs the extension, it will get to it
                  // via one of the other possible instances being 
                  // extended
                  //
                  e2 = vertex2->numEdges;
               }
            }
         }
      }
      MarkInstanceEdges(instance, g2, FALSE);
      instanceListNode = instanceListNode->next;
   }
   //
   // If no new extensions, return what we have so far
   //
   if ((newInstanceList->head == NULL) && (noExtensions))
   {
      instanceListNode = instanceList->head;
      if (instanceListNode->instance != NULL)
      {
         while (instanceListNode != NULL)
         {
            InstanceListInsert(instanceListNode->instance, newInstanceList,
                               TRUE);
            instanceListNode = instanceListNode->next;
         }
      }
   }

   FreeInstanceList(instanceList);
   return newInstanceList;
}
