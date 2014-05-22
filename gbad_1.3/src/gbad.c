//---------------------------------------------------------------------------
// gbad.c
//
// GBAD functions.
//
// GBAD, version 1.3
//---------------------------------------------------------------------------

#include "subdue.h"

//---------------------------------------------------------------------------
// NAME: FindTrueBestSubstructure (GBAD-MDL)
//
// INPUTS: (SubList *subList) - list of substructures to check
//         (Parameters *parameters)
//
// RETURN: (SubList) - list of substructures in anomalous order
//
// PURPOSE:  The purpose of this routine is to find the true best
// substructures.  It has been assumed that a threshold > 0
// was chosen, which may result in the best substructure
// pattern not being the normative pattern.  So, the idea is to loop
// through the list of substructures and their instances, and find
// the true one by setting the threshold to 0 (temporarily).
//
// NOTE: Currently, the way the code is written below, the true best
//       substructure is the most frequent one.
//
// NOTE: What we need to do is create a substructure list out of the instances
//       of the best substructure, and use that list as the new list of
//       of substructures.
//
// NOTE: Since this is currently only looking at the first substructure
//       in the list (i.e. the best one), why not just pass in the
//       substructure and not the list?
//---------------------------------------------------------------------------
SubList* FindTrueBestSubstructure(SubList *subList, Parameters *parameters)
{
   SubListNode *subListNode;
   Substructure *currentBestSub;
   Substructure *newSub;
   SubList *newSubList;
   SubList *bestSubList;
   SubListNode *newSubListNode = NULL;
   Instance *currentInstance;
   InstanceList *currentInstanceList = NULL;
   InstanceListNode *currentInstanceListNode;
   InstanceListNode *bestInstanceListNode;
   Substructure *bestSub = NULL;
   Instance *bestInstance = NULL;
   ULONG currentInstanceListIndex = 0;
   ULONG maxInstances = 0;
   ULONG bestSubNumVertices = 0;
   ULONG bestSubNumEdges = 0;

   Graph *posGraph = parameters->posGraph;
   LabelList *labelList = parameters->labelList;

   //
   // First, take all instances of the best (first) substructure, and
   // create a new substructure list.  From this list, find the best
   // substructure.
   //
   subListNode = subList->head;
   parameters->threshold = 0.0;
   //
   // NOTE: As mentioned above, this only looks at the FIRST
   //       substructure.  In order to look at all of them, should add a
   //       while loop here.
   //
   currentBestSub = subListNode->sub;
   newSubList = AllocateSubList();
   // Get instances for substructure
   currentInstanceList = currentBestSub->instances;
   // Now loop through the instances and create substructure and their
   // instances (out of the original instance list)
   currentInstanceListNode = currentInstanceList->head;
   while (currentInstanceListNode != NULL)
   {
      currentInstance = currentInstanceListNode->instance;
      if (currentInstance->minMatchCost != 0.0) {
            newSub = CreateSubFromInstance(currentInstance, posGraph);
            if (! MemberOfSubList(newSub, newSubList, labelList))
            {
               AddPosInstancesToSub(newSub, currentInstance, currentInstanceList,
                                     parameters,currentInstanceListIndex);
               // add newSub to head of newSubList list
               newSubListNode = AllocateSubListNode(newSub);
               newSubListNode->next = newSubList->head;
               newSubList->head = newSubListNode;
            } else FreeSub(newSub);
      }
      currentInstanceListNode = currentInstanceListNode->next;
      currentInstanceListIndex++;
   }
   //
   // Now, find the one that is the most frequent
   //
   // NOTE: This could be rewritten to create a sorted list of subs
   //
   subListNode = newSubList->head;
   while (subListNode != NULL)
   {
      EvaluateSub(subListNode->sub, parameters);
      if (subListNode->sub->numInstances > maxInstances)
      {
         bestSub = CopySub(subListNode->sub);
         maxInstances = bestSub->numInstances;
      }
      subListNode = subListNode->next;
   }
   maxInstances = 0;
   FreeSubList(newSubList);
   parameters->threshold = parameters->mdlThreshold;

   printf("Normative Pattern:\n");
   PrintSub(bestSub,parameters);
   printf("\n");

   //
   // Second, we want to change the best substructure to be the new best
   // substructure, BUT, include all of the original instances SO that
   // we can get the proper cost of transformation
   //
   // NOTE: We MUST exchange the final substructure list with the true
   //       best one, otherwise we will not get the graph compressed
   //       by the correct substructure. However, note that this code
   //       currently only returns the top best substructure.
   //
   if (currentInstanceList != NULL)
   {
      bestSub->instances = AllocateInstanceList();
      bestInstanceListNode = currentInstanceList->head;
      bestSubNumVertices = bestInstanceListNode->instance->numVertices;
      bestSubNumEdges = bestInstanceListNode->instance->numEdges;
      while (bestInstanceListNode != NULL)
      {
         if (bestInstanceListNode->instance != NULL)
         {
            bestInstance = bestInstanceListNode->instance;
            //
            // Only keep instances that are the same size as the best
            // substructure (the cost of transformation could result
            // in matching instances that are not the same size)
            //
            // NOTE: Does this assume that the prevalent instance is at the head?
            //
            if ((bestInstance->numVertices == bestSubNumVertices) &&
                (bestInstance->numEdges == bestSubNumEdges))
               InstanceListInsert(bestInstance, bestSub->instances, FALSE);
            bestSub->numInstances++;
            bestInstanceListNode = bestInstanceListNode->next;
         }
      }
   }

   //
   // Reset number of instances and list of instances
   //
   bestSubList = AllocateSubList();

   // Put best substructure on list (will be the ONLY one)
   SubListInsert(bestSub, bestSubList, 0, TRUE, labelList);

   return bestSubList;
}


//---------------------------------------------------------------------------
// NAME: FlagAnomalousVerticesAndEdges (GBAD)
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
//---------------------------------------------------------------------------
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
   }
}


//---------------------------------------------------------------------------
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
// NOTE: This routine assumes that CreateSubstructureAndInstancesList has been 
//       called first.
//---------------------------------------------------------------------------
void PrintProbabilisticAnomalies(InstanceList *instanceList,
                                 Parameters *parameters)
{
   InstanceListNode *firstInstanceListNode;
   ULONG i;

   Graph *posGraph = parameters->posGraph;

   //
   // Print all instances with the lowest probability that are equal to or
   // below the user-specified threshold.
   //
   firstInstanceListNode = instanceList->head;
   if (firstInstanceListNode != NULL)
      printf("Anomalous Instance(s):\n");
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
               if (strncmp(parameters->labelList->labels[posGraph->vertices[firstInstanceListNode->instance->vertices[i]].label].labelValue.stringLabel,"SUB_",4))
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
}


//---------------------------------------------------------------------------
// NAME: CopySubList (GBAD-P)
//
// INPUTS: (SubList *subList) - list of substructures
//         (Parameters *parameters)
//
// RETURN: (SubList) - copied list of substructures
//
// PURPOSE:  This routine creates a copy of the specified substructure list.
//---------------------------------------------------------------------------
SubList* CopySubList(SubList *subList, Parameters *parameters)
{
   Substructure *newSub = NULL;
   SubList *newSubList = NULL;
   SubListNode *subListNode = NULL;
   Instance *instance = NULL;
   InstanceList *instanceList = NULL;
   InstanceListNode *instanceListNode = NULL;

   newSubList = AllocateSubList();
   subListNode = subList->head;
   while (subListNode != NULL)
   {
      newSub = CopySub(subListNode->sub);
      instanceList = subListNode->sub->instances;
      newSub->instances = AllocateInstanceList();
      instanceListNode = instanceList->head;
      while (instanceListNode != NULL)
      {
         if (instanceListNode->instance != NULL)
         {
            instance = instanceListNode->instance;
            InstanceListInsert(instance, newSub->instances, FALSE);
            instanceListNode = instanceListNode->next;
         }
      }
      SubListInsert(newSub, newSubList, 0, TRUE, parameters->labelList);
      subListNode = subListNode->next;
   }
   return newSubList;
}


//---------------------------------------------------------------------------
// NAME: PrintMaxPartialAnomalousInstances (GBAD-MPS)
//
// INPUTS: (SubList *subList) - list of substructures
//         (Parameters *parameters)
//
// RETURN: (Sublist *)
//
// PURPOSE:  This routine looks at all parent substructures and their instances
// (in this implementation, "parents" are found by increasing the nsubs
// parameter).  If any instance is a partial match (i.e. all of its
// vertices and edges are included in the best substructure), and it does
// not match with any of the instances of the best substructure (such that
// it would eventually extend to the best substructure), then output the
// one with the lowest (cost of transformation * frequency).
//---------------------------------------------------------------------------
SubList* PrintMaxPartialAnomalousInstances(SubList *subList, Parameters *parameters)
{
   SubListNode *subListNode = NULL;
   InstanceList *bestInstanceList = NULL;
   InstanceListNode *instanceListNode = NULL;
   Instance *parentInstance = NULL;
   Instance *bestInstance = NULL;
   Graph *bestGraph = NULL;
   Substructure *bestSub = NULL;
   Graph *anomalousInstanceGraph = NULL;
   InstanceList *parentInstanceList = NULL;
   InstanceListNode *parentInstanceListNode = NULL;
   double matchCost;
   BOOLEAN foundOne = FALSE;
   ULONG bestNumVertices;
   ULONG bestNumEdges;
   InstanceList *anomalousInstances = NULL;
   ULONG posEgNo;
   ULONG numPosEgs = parameters->numPosEgs;
   ULONG *posEgsVertexIndices = parameters->posEgsVertexIndices;
   ULONG frequencyCount = 0;
   Graph *otherInstanceGraph = NULL;
   InstanceListNode *otherInstanceListNode = NULL;
   double minAnomalousValue = MAX_DOUBLE;

   Graph *graph = parameters->posGraph;
   LabelList *labelList = parameters->labelList;

   anomalousInstances = AllocateInstanceList();
   subListNode = subList->head;
   bestGraph = subListNode->sub->definition;
   bestSub = subListNode->sub;
   bestInstance = subListNode->sub->instances->head->instance;
   bestInstanceList = subListNode->sub->instances;
   bestNumVertices = bestInstance->numVertices;
   bestNumEdges = bestInstance->numEdges;

   ULONG newInstanceListIndex = 0;

   Substructure *newSub = NULL;
   SubList *newSubList = NULL;
   newSubList = AllocateSubList();

   //
   // Now loop over the other best substructures and their instances
   //
   subListNode = subListNode->next;
   //
   while (subListNode != NULL)
   {
      //
      // What we want is to find a substructure that completely
      // overlaps with the best substructure.  Then, find all of its
      // instances that do NOT overlap with any of the best instances
      // (as they might eventually extend to that instance).
      //
      // First, see if substructures match within the given threshold
      //
      // NOTE: The cost used here is not calculated as the cost of 
      //       transformation times the frequency -- thus a lot more
      //       substructures are going to be considered.
      //
      GraphMatch(bestGraph,subListNode->sub->definition,labelList,
                 MAX_DOUBLE, & matchCost, NULL);
      if (matchCost <= parameters->maxAnomalousScore)
      {
         //
         // Second, loop over its instances and only consider those
         // that do not overlap with any instances of the best
         // substructure.
         //
         parentInstanceList = subListNode->sub->instances;
         parentInstanceListNode = parentInstanceList->head;
         while (parentInstanceListNode != NULL)
         {
            if (parentInstanceListNode->instance != NULL)
            {
               parentInstance = parentInstanceListNode->instance;
               //
               // See if the instance overlaps with any of the
               // best substructure instances
               //
               foundOne = InstanceListOverlap(parentInstance,bestInstanceList);
               //
               // Search list of already discoved anomalous instances, and if
               // this instance is already in the list, skip it.
               //
               instanceListNode = anomalousInstances->head;
               while ((instanceListNode != NULL) && !foundOne)
               {
                  foundOne = InstanceMatch(instanceListNode->instance,parentInstance);
                  if (!foundOne)
                     instanceListNode = instanceListNode->next;
               }
               //
               // Now, calculate value of performing a subgraph
               // isomorphism (i.e. value for transformation) to be used as
               // its anomalous value
               //
               if (!foundOne) {
                  anomalousInstanceGraph = InstanceToGraph
                                             (parentInstance, graph);
                  GraphMatch(bestGraph,anomalousInstanceGraph,labelList,
                             MAX_DOUBLE, & matchCost, NULL);
                  //
                  // For now, just set the anomalous value to the match... will
                  // need to check later for frequency (i.e., how many
                  // other instances of this same structure exist).
                  //
                  parentInstance->mpsAnomalousValue = (double) matchCost; 
                  if ((parentInstance->mpsAnomalousValue <= parameters->maxAnomalousScore) &&
                      (parentInstance->mpsAnomalousValue >= parameters->minAnomalousScore))
                     InstanceListInsert(parentInstance,anomalousInstances,TRUE);
                  FreeGraph(anomalousInstanceGraph);
               }
            }
            parentInstanceListNode = parentInstanceListNode->next;
         }
      }
      subListNode = subListNode->next;
   }
   //
   // Now loop through the anomalous instances (if any) and calculate the final
   // anomalous value for each of them, and print them out
   //
   instanceListNode = anomalousInstances->head;
   while (instanceListNode != NULL)
   {
       anomalousInstanceGraph = InstanceToGraph(instanceListNode->instance, 
                                                graph);
       otherInstanceListNode = anomalousInstances->head;
       frequencyCount = 0.0;
       while (otherInstanceListNode != NULL)
       { 
          otherInstanceGraph = InstanceToGraph(otherInstanceListNode->instance, 
                                               graph);
          GraphMatch(otherInstanceGraph,anomalousInstanceGraph,labelList,
                     MAX_DOUBLE, & matchCost, NULL);
          if (matchCost == 0.0)
             frequencyCount++;
          otherInstanceListNode = otherInstanceListNode->next;
          FreeGraph(otherInstanceGraph);
       }
       instanceListNode->instance->mpsAnomalousValue = 
          (double) instanceListNode->instance->mpsAnomalousValue * 
          (double) frequencyCount; 
       if (instanceListNode->instance->mpsAnomalousValue < minAnomalousValue)
          minAnomalousValue = instanceListNode->instance->mpsAnomalousValue;
       instanceListNode = instanceListNode->next;
       FreeGraph(anomalousInstanceGraph);
   }
   //
   // Now, just output the anomaly (or anomalies) with the best score
   //
   instanceListNode = anomalousInstances->head;
   if (instanceListNode == NULL)
      printf("Anomalous Instances:  NONE\n\n");
   else
   {
      //
      // If a non-zero threshold was specified, see if we can figure out
      // if any labels were modified (a.k.a. the MDL approach to finding
      // anomalies).
      //
      if (parameters->threshold > 0.0)
      {
         subListNode = subList->head;
         bestGraph = subListNode->sub->definition;
         bestSub = subListNode->sub;
         FlagAnomalousVerticesAndEdges(anomalousInstances, graph, bestSub, 
                                       parameters);
      }
      //
      printf("Anomalous Instances:\n\n");
      while (instanceListNode != NULL)
      {
         if (instanceListNode->instance->mpsAnomalousValue == minAnomalousValue)
         {
            // In the case of the MPS algorithm, the entire instance structure
            // is anomalous
            if (parameters->mps) {
	       ULONG i;
               instanceListNode->instance->numAnomalousVertices = instanceListNode->instance->numVertices;
	       for (i=0; i<instanceListNode->instance->numVertices; i++)
	          instanceListNode->instance->anomalousVertices[i] = instanceListNode->instance->vertices[i];
               instanceListNode->instance->numAnomalousEdges = instanceListNode->instance->numEdges;
	       for (i=0; i<instanceListNode->instance->numEdges; i++)
	          instanceListNode->instance->anomalousEdges[i] = instanceListNode->instance->edges[i];
            }
            posEgNo = InstanceExampleNumber(instanceListNode->instance,
                                            posEgsVertexIndices, 
                                            numPosEgs);
            printf(" from positive example %lu:\n", posEgNo);
            PrintAnomalousInstance(instanceListNode->instance, graph, 
	                           parameters);
            printf("   (max_partial_substructure anomalous value = ");
            printf("%f )\n",instanceListNode->instance->mpsAnomalousValue);
            newSub = CreateSubFromInstance(instanceListNode->instance, graph);
            AddPosInstancesToSub(newSub, instanceListNode->instance, anomalousInstances,
                                 parameters,newInstanceListIndex);
            SubListInsert(newSub, newSubList, 0, TRUE, parameters->labelList);
         }
         instanceListNode = instanceListNode->next;
      }
   }
   return newSubList;
}


//---------------------------------------------------------------------------
// NAME: StoreAnomalousEdge (GBAD)
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
//---------------------------------------------------------------------------

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


//---------------------------------------------------------------------------
// NAME: PrintAnomalousVertex (GBAD)
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
//---------------------------------------------------------------------------

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


//---------------------------------------------------------------------------
// NAME: PrintAnomalousEdge (GBAD)
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
//---------------------------------------------------------------------------

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


//---------------------------------------------------------------------------
// NAME: PrintAnomalousInstance (GBAD)
//
// INPUTS: (Instance *instance) - instance to print
//         (Graph *graph) - graph containing instance
//         (LabelList *labelList) - labels from graph
//
// RETURN: (void)
//
// PURPOSE: Print given instance.
//---------------------------------------------------------------------------

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


//---------------------------------------------------------------------------
// NAME: PrintAnomalousInstanceList (GBAD)
//
// INPUTS: (InstanceList *instanceList) - list of instances
//         (Graph *graph) - graph containing instances
//         (LabelList *labelList) - labels used in input graph
//
// RETURN: (void)
//
// PURPOSE: Print array of instances.
//---------------------------------------------------------------------------

void PrintAnomalousInstanceList(InstanceList *instanceList, Graph *graph,
                                Parameters *parameters)
{
   ULONG i = 0;
   InstanceListNode *instanceListNode;

   if (instanceList != NULL)
   {
      instanceListNode = instanceList->head;
      while (instanceListNode != NULL)
      {
         printf("\n  Instance %lu:\n", i + 1);
         PrintAnomalousInstance(instanceListNode->instance, graph, parameters);
         //
         instanceListNode = instanceListNode->next;
         i++;
      }
   }
}


//---------------------------------------------------------------------------
// NAME: PrintAnomalousPosInstanceList (GBAD)
//
// INPUTS: (Substructure *sub) - substructure containing positive instances
//         (Parameters *parameters)
//
// RETURN: (void)
//
// PURPOSE: Print array of sub's positive instances.
//---------------------------------------------------------------------------

void PrintAnomalousPosInstanceList(Substructure *sub, Parameters *parameters)
{
   ULONG i;
   ULONG posEgNo;
   InstanceListNode *instanceListNode;

   // parameters used
   Graph *posGraph = parameters->posGraph;
   ULONG numPosEgs = parameters->numPosEgs;
   ULONG *posEgsVertexIndices = parameters->posEgsVertexIndices;
   LabelList *labelList = parameters->labelList;
   double thresholdLimit;
   Graph *instanceGraph;
   double matchCost;
   double threshold = parameters->threshold;
   //

   if (sub->instances != NULL)
   {
      instanceListNode = sub->instances->head;
      i = 1;
      while (instanceListNode != NULL)
      {
         printf("\n  Instance %lu", i);
         if (numPosEgs > 1)
         {
            posEgNo = InstanceExampleNumber(instanceListNode->instance,
                                            posEgsVertexIndices, numPosEgs);
            printf(" in positive example %lu:\n", posEgNo);
         }
         else
            printf(":\n");
         PrintAnomalousInstance(instanceListNode->instance, posGraph, parameters);
         //
         // Need to create a graph from the instance, compare it to
         // posGraph, and print out the cost.
         //
         thresholdLimit = threshold *
                          (instanceListNode->instance->numVertices +
                           instanceListNode->instance->numEdges);
         instanceGraph = InstanceToGraph(instanceListNode->instance, posGraph);
         GraphMatch(sub->definition, instanceGraph, labelList,
                                 thresholdLimit, & matchCost, NULL);
         // not testing for TRUE/FALSE because it had to be TRUE at the end...
         instanceListNode = instanceListNode->next;
         i++;
      }
   }
}


//---------------------------------------------------------------------------
// NAME: PrintAnomalousNegInstanceList (GBAD)
//
// INPUTS: (Substructure *sub) - substructure containing negative instances
//         (Parameters *parameters)
//
// RETURN: (void)
//
// PURPOSE: Print array of sub's negative instances.
//---------------------------------------------------------------------------

void PrintAnomalousNegInstanceList(Substructure *sub, Parameters *parameters)
{
   ULONG i;
   ULONG negEgNo;
   InstanceListNode *instanceListNode;

   // parameters used
   Graph *negGraph = parameters->negGraph;
   ULONG numNegEgs = parameters->numNegEgs;
   ULONG *negEgsVertexIndices = parameters->negEgsVertexIndices;

   if (sub->negInstances != NULL)
   {
      instanceListNode = sub->negInstances->head;
      i = 1;
      while (instanceListNode != NULL)
      {
         printf("\n  Instance %lu", i);
         if (numNegEgs > 1)
         {
            negEgNo = InstanceExampleNumber(instanceListNode->instance,
                                            negEgsVertexIndices, numNegEgs);
            printf(" in negative example %lu:\n", negEgNo);
         }
         else
            printf(":\n");
         PrintAnomalousInstance(instanceListNode->instance, negGraph,
                                parameters);
         //
         instanceListNode = instanceListNode->next;
         i++;
      }
   }
}

//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------

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

//---------------------------------------------------------------------------
// NAME: FindAnomalousSubstructure (GBAD-MDL)
//
// INPUTS: (SubList *subList) - list of substructures
//         (Parameters *parameters)
//
// RETURN: (Sublist *)
//
// PURPOSE:  This routine compares the best substructure (i.e., the first one
// in the supplied list of substructures) to all of the substructures in the
// list, finding the one that is "closest" to the best.  The "closest" one is
// the substructure that is (1) of the same size as the best substructure, and
// (2) deviates from the best substructure by the specified mdlThreshold.  This
// substructure is then considered the most anomalous, and will become the new
// "best substructure".
//---------------------------------------------------------------------------
SubList* FindAnomalousSubstructure(SubList *subList, Parameters *parameters)
{
   SubListNode *subListNode = NULL;
   SubListNode *bestSubListNode = NULL;
   Substructure *bestSub = NULL;
   double matchCost;
   double minMatchCost = MAX_DOUBLE;
   double matchThreshold;
   double anomalousValue = 0.0;
   ULONG frequency = 0;
   ULONG bestSubNumVertices;
   ULONG bestSubNumEdges;
   BOOLEAN foundOne = FALSE;
   Instance *instance = NULL;
   InstanceList *instanceList = NULL;
   InstanceList *bestSubInstanceList = NULL;
   InstanceListNode *instanceListNode = NULL;
   InstanceList *anomInstanceList = NULL;
   SubList *newSubList = NULL;
   newSubList = AllocateSubList();

   //
   // First, save the best substructure that will need to be
   // compared to.
   //
   subListNode = subList->head;
   bestSubListNode = subList->head;
   bestSubNumVertices = subListNode->sub->definition->numVertices;
   bestSubNumEdges = subListNode->sub->definition->numEdges;
   bestSub = CopySub(subListNode->sub);
   bestSub->numInstances = 0;
   bestSub->numExamples = 0;

   printf("Normative Pattern:\n");
   PrintSub(bestSub,parameters);
   printf("\n");

   bestSubInstanceList = bestSubListNode->sub->instances;
   //
   // Now loop over the other substructures and see which one is "closest"
   // to matching the best substructure (by default, it should never match
   // exactly - otherwise, it would have been in the best substructure).
   //
   subListNode = subListNode->next;
   //
   while (subListNode != NULL)
   {
      //maxVertices = sub->definition->numVertices;
      //mapping = (VertexMap *) malloc(sizeof(VertexMap) * maxVertices);
      // make sure we are only comparing substructures of the same size...
      if ((subListNode->sub->definition->numVertices == bestSubNumVertices) &&
          (subListNode->sub->definition->numEdges == bestSubNumEdges))
      {
         matchCost = InexactGraphMatch(subListNode->sub->definition, 
	                               bestSub->definition,
                                       parameters->labelList, 
                                       MAX_DOUBLE,
				       NULL);
	 matchThreshold = matchCost / (bestSubNumVertices + bestSubNumEdges);
         //
         // If the cost of transforming this substructure into the original best
         // substructure is less than the user specified mdlThreshold, and it
	 // is less then the user specified maxAnomalousScore, and
         // it is less than the previous best (lowest) transformation cost,
         // then mark this substructure as best (for now).
         //
         if ((matchThreshold <= parameters->mdlThreshold) &&
	     (matchCost <= parameters->maxAnomalousScore) &&
	     (matchCost < minMatchCost))
         {
	    foundOne = TRUE;
	    minMatchCost = matchCost;
	    //
	    // Substructure should always be the normative pattern -
	    // but we need to attach only the anomalous instances to the
	    // normative substructure
	    //
	    frequency = subListNode->sub->numInstances;
            instanceList = subListNode->sub->instances;
            anomInstanceList = AllocateInstanceList();
            instanceListNode = instanceList->head;
            while (instanceListNode != NULL)
            {
               if (instanceListNode->instance != NULL)
               {
                  instance = instanceListNode->instance;
                  //
                  // See if the anomalous instance overlaps with any of the
                  // best substructure instances, and if it does not, add
		  // it to the list of anomalous instances
                  //
                  if (!InstanceListOverlap(instance,bestSubInstanceList))
		  {
		     instance->minMatchCost = matchCost;
                     //
                     // Calculate anomalous value of this instance
                     //
                     anomalousValue =
                        (double) instance->minMatchCost *
                        (double) frequency;
                     instance->infoAnomalousValue = anomalousValue;
                     //
                     InstanceListInsert(instance, anomInstanceList, FALSE);
                  }
                  instanceListNode = instanceListNode->next;
               }
            }
         }
	 //
	 // If they are equal in "anomalousness", then add its instances to the (what 
	 // should be an already existing) list of instances for the normative
	 // pattern), so that we are returning a list of all anomalous instances
	 // that have the same value of anomalousness.
	 //
         else if ((matchThreshold <= parameters->mdlThreshold) &&
	          (matchCost <= parameters->maxAnomalousScore) &&
	          (matchCost == minMatchCost))
         {
	    frequency = subListNode->sub->numInstances;
            instanceList = subListNode->sub->instances;
            instanceListNode = instanceList->head;
            while (instanceListNode != NULL)
            {
               if (instanceListNode->instance != NULL)
               {
                  instance = instanceListNode->instance;
                  //
                  // See if the anomalous instance overlaps with any of the
                  // best substructure instances, and if it does not, add
		  // it to the list of anomalous instances
                  //
                  if (!InstanceListOverlap(instance,bestSubInstanceList))
		  {
		     instance->minMatchCost = matchCost;
                     //
                     // Calculate anomalous value of this instance
                     //
                     anomalousValue =
                        (double) instance->minMatchCost *
                        (double) frequency;
                     instance->infoAnomalousValue = anomalousValue;
                     //
                     InstanceListInsert(instance, anomInstanceList, FALSE);
                  }
                  instanceListNode = instanceListNode->next;
               }
            }
	 }
      }
      subListNode = subListNode->next;
   }

   //
   // Finally, add all of the anomalous instances to the substructure that is
   // providing the normative pattern.
   //
   if (anomInstanceList != NULL)
   {
      bestSub->instances = AllocateInstanceList();
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
   }

   newSubList = AllocateSubList();
   SubListInsert(bestSub, newSubList, 0, FALSE, parameters->labelList);

   if (!foundOne)
      return NULL;
   else
      return newSubList;
}


//---------------------------------------------------------------------------
// NAME: PrintAnomalousInstances (GBAD-MDL)
//
// INPUTS: (SubList *subList) - list of substructures with possible anomalous
//                              instances
//         (Parameters *parameters)
//
// RETURN: (void)
//
// PURPOSE:  This routine prints the anomalous instance(s) found in each
// substructure.
//
// NOTE: The way the code is currently written, it is only looking for the
//       anomalous instances from the best substructure.
//---------------------------------------------------------------------------
void PrintAnomalousInstances(SubList *subList, Parameters *parameters)
{
   SubListNode *subListNode = NULL;
   InstanceListNode *firstInstanceListNode;
   ULONG minimumValue = MAX_UNSIGNED_LONG;
   InstanceList *anomalousInstanceList = NULL;

   Graph *posGraph = parameters->posGraph;

   //
   // First, loop through the list of candidate anomalous substructures,
   // and find the minimum anomalous score
   //
   firstInstanceListNode = subList->head->sub->instances->head;
   while (firstInstanceListNode != NULL)
   {
      if ((firstInstanceListNode->instance->infoAnomalousValue <= minimumValue) && 
          (firstInstanceListNode->instance->infoAnomalousValue != 0))
      {
         minimumValue = firstInstanceListNode->instance->infoAnomalousValue;
      }
      firstInstanceListNode = firstInstanceListNode->next;
   }
   //
   // Then, go back through the list of candidate anomalous substructures,
   // and save the ones that matched the minimum anomalous score
   //
   anomalousInstanceList = AllocateInstanceList();
   firstInstanceListNode = subList->head->sub->instances->head;
   while (firstInstanceListNode != NULL)
   {
      if ((firstInstanceListNode->instance->infoAnomalousValue == minimumValue) && 
          (firstInstanceListNode->instance->infoAnomalousValue != 0))
      {
         InstanceListInsert(firstInstanceListNode->instance,
                            anomalousInstanceList,FALSE);
      }
      firstInstanceListNode = firstInstanceListNode->next;
   }
   //
   // Now, loop through the "final" list of anomalous instances, and
   // output those that meet the user-specified criteria
   //
   if (anomalousInstanceList != NULL)
   {
      subListNode = subList->head;
      FlagAnomalousVerticesAndEdges(anomalousInstanceList, posGraph,
                                    subListNode->sub, parameters);
      //
      // Print all instances within the user-specified thresholds
      //
      // NOTE:  The maxAnomalousScore is checked in FindAnomalousSubstructure,
      //        so need to re-think about the redundancy of the logic here...
      //
      firstInstanceListNode = anomalousInstanceList->head;
      if (firstInstanceListNode != NULL)
      {
         printf("Anomalous Instance(s):\n");
         while (firstInstanceListNode != NULL)
         {
            if ((firstInstanceListNode->instance->infoAnomalousValue <=
                 parameters->maxAnomalousScore) &&
                (firstInstanceListNode->instance->infoAnomalousValue >=
                 parameters->minAnomalousScore))
            {
               //
               // If the user specifies the -minOnly parameters, only print
               // the structures that are the best (i.e. most anomalous)
               //
	       // NOTE:  Again, because of the way FindAnomalousSubstructure
	       //        works, I think you would only get the minimum
	       //        anomalous instances anyway...
	       //
               if ((parameters->minOnly) &&
                   (firstInstanceListNode->instance->infoAnomalousValue > minimumValue))
               {
                  // skip this structure
               }
               else
               {
                  printf("\n");
                  ULONG posEgNo;
                  ULONG numPosEgs = parameters->numPosEgs;
                  ULONG *posEgsVertexIndices = parameters->posEgsVertexIndices;
                  posEgNo = InstanceExampleNumber(firstInstanceListNode->instance,
                                                  posEgsVertexIndices,
                                                  numPosEgs);
                  printf(" from positive example %lu:\n", posEgNo);
                  PrintAnomalousInstance(firstInstanceListNode->instance, posGraph,
                                         parameters);
                  printf("    (information_theoretic anomalous value = %f )\n",
                         firstInstanceListNode->instance->infoAnomalousValue);
               }
               firstInstanceListNode = firstInstanceListNode->next;
            }
         }
      } else {
         printf("Anomalous Instances:  NONE.\n");
      }
   } else {
      printf("Anomalous Instances:  NONE.\n");
   }
}

//---------------------------------------------------------------------------
// NAME: CreateSubstructureAndInstancesList (GBAD-P)
//
// INPUTS: (SubList *subList) - list of substructures
//         (Parameters *parameters)
//
// RETURN: (SubList) - modified list of substructures, with possible
//                     anomalous instances attached
//
// PURPOSE: The purpose of this routine is to return the normative pattern
// as the best substructure AND set all of its instances to all of the
// most anomalous instances.  This way, in the next iteration, all of the
// anomalous instances will be compressed so that we can look at the extensions
// to the anomalous instances.  It is important to note make sure that the best 
// substructure returned is the one containing the previously compressed 
// substructure.  The reason for this is to ensure that the basis for the 
// anomaly found on this iteration is still grounded in the original instance 
// (i.e. an extension of that instance).
//
//---------------------------------------------------------------------------
SubList* CreateSubstructureAndInstancesList(SubList *subList, Parameters *parameters)
{
   SubListNode *subListNode;
   SubList *bestSubList;
   Substructure *bestSub = NULL;
   Instance *instance;
   Instance *anomInstance;
   InstanceList *instanceList = NULL;
   InstanceList *anomInstanceList = NULL;
   InstanceListNode *instanceListNode;
   InstanceListNode *anomInstanceListNode;
   ULONG i;
   ULONG minNumInstances = MAX_UNSIGNED_LONG;
   ULONG totalNumInstances = 0;
   BOOLEAN found = FALSE;

   char subLabelString[TOKEN_LEN];
   sprintf(subLabelString, "%s_%lu", SUB_LABEL_STRING, (parameters->currentIteration-1));

   //
   // First, find the normative substructure and print it...
   //
   ULONG maxNumInstances = 0;
   subListNode = subList->head;
   while (subListNode != NULL)
   {
      for (i=0;i<subListNode->sub->definition->numVertices;i++)
      {
         if (!strncmp(parameters->labelList->labels[subListNode->sub->definition->vertices[i].label].labelValue.stringLabel,subLabelString,5))
         {
            totalNumInstances = totalNumInstances + subListNode->sub->numInstances;
            if (subListNode->sub->numInstances > maxNumInstances)
            { // possible normative substructure...
               bestSub = CopySub(subListNode->sub);
               maxNumInstances = subListNode->sub->numInstances;
            }
         }
      }
      subListNode = subListNode->next;
   }
   if (bestSub != NULL) 
   {
      bestSub->numInstances = totalNumInstances;
      printf("Normative Pattern:\n");
      PrintSub(bestSub,parameters);
      printf("\n");
   } else
      return NULL;

   //
   // Loop through all of the substructures, and find the substructures with
   // the fewest number of instances.
   //
   // NOTE:  Only look at substructures that contain "SUB_" in its substructure.
   //
   subListNode = subList->head;
   while (subListNode != NULL)
   {
      for (i=0;i<subListNode->sub->definition->numVertices;i++)
      {
         if (!strncmp(parameters->labelList->labels[subListNode->sub->definition->vertices[i].label].labelValue.stringLabel,subLabelString,5))
         {
            if (subListNode->sub->numInstances < minNumInstances)
            {   // possible anomalous substructure - get its instances...
               minNumInstances = subListNode->sub->numInstances;
               instanceList = subListNode->sub->instances;
               anomInstanceList = AllocateInstanceList();
               instanceListNode = instanceList->head;
               while (instanceListNode != NULL)
               {
                  if (instanceListNode->instance != NULL)
                  {
		     found = FALSE;
                     instance = instanceListNode->instance;
                     anomInstanceListNode = anomInstanceList->head;
		     while ((anomInstanceListNode != NULL) && !found)
		     {
                        if (anomInstanceListNode->instance != NULL)
			{
                           anomInstance = anomInstanceListNode->instance;
			   if (InstanceMatch(instance,anomInstance))
			      found = TRUE;
			}
                        anomInstanceListNode = anomInstanceListNode->next;
		     }
		     if ((anomInstanceListNode == NULL) || !found)
		     {
                        instance->probAnomalousValue = (double)minNumInstances / 
		                                       (double)bestSub->numInstances;
                        InstanceListInsert(instance, anomInstanceList, FALSE);
		     }
                  }
                  instanceListNode = instanceListNode->next;
               }
	    // also want to keep instances of a substructure that has an equal
	    // number of instances (could be more than one anomalous instance)
            } else if (subListNode->sub->numInstances == minNumInstances) {
               instanceList = subListNode->sub->instances;
               instanceListNode = instanceList->head;
               while (instanceListNode != NULL)
               {
                  if (instanceListNode->instance != NULL)
                  {
                     //
                     // See if the anomalous instance is already inserted
                     // into the anomalous instances list, and if it does not,
	   	     // add it to the list of anomalous instances
                     //
		     found = FALSE;
                     instance = instanceListNode->instance;
                     anomInstanceListNode = anomInstanceList->head;
		     while ((anomInstanceListNode != NULL) && !found)
		     {
                        if (anomInstanceListNode->instance != NULL)
			{
                           anomInstance = anomInstanceListNode->instance;
			   if (InstanceMatch(instance,anomInstance))
			      found = TRUE;
			}
                        anomInstanceListNode = anomInstanceListNode->next;
		     }
		     if (!found)
		     {
                        instance->probAnomalousValue = (double)minNumInstances / 
      		                                       (double)bestSub->numInstances;
                        InstanceListInsert(instance, anomInstanceList, FALSE);
      		     }
                  }
                  instanceListNode = instanceListNode->next;
               }
	    }
         }
      }
      subListNode = subListNode->next;
   }

   //
   // Finally, add all of the anomalous instances to the substructure that is
   // providing the normative pattern.
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

   //
   // Create new best substructure list
   //
   bestSubList = AllocateSubList();

   // Put best substructure and instances on list (will be the ONLY one)
   SubListInsert(bestSub, bestSubList, 0, FALSE, parameters->labelList);

   return bestSubList;
}
