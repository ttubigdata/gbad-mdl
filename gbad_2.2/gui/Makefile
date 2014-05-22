JC = javac
JC_FLAG = -source 1.5 -target 1.5

JR = jar
JR_FLAG = -cvmf
MANIFEST = manifest.txt

SRC = Main.java CustomFileFilter.java GUI.java GraphText.java JGraphViz.java Parameters.java UpdateGBADCount.java DraggableJScrollPane.java GraphDisplay.java GraphVisualization.java GBADMenu.java GraphResults.java Images.java GBADPreferences.java StartProcess.java

OBJ = CancelActionListener.class CreateGraphTask.class CustomFileFilter.class DraggableJScrollPane.class GBADMenu.class GBADPreferences.class GBADProcessMonitor.class GUI.class GraphDisplay.class GraphResults.class GraphText.class GraphVisualization.class GreaterThanOneKeyListener.class Images.class JGraphViz.class LimitKeyListener.class ListCellRenderer.class Main.class MaxKeyListener.class MinKeyListener.class OneToFiveKeyListener.class Parameters.class PathMouseListener.class PercentKeyListener.class PostiveIntegerKeyListener.class ResizeGraphTask.class RestoreDefaultsActionListener.class RestoreNamedDefaultActionListener.class SaveActionListener.class StartProcess.class StateChangeListener.class UpdateGBADCount.class

PROG = GUI.jar

all: $(PROG)

$(PROG): $(OBJ)
	$(JR) $(JR_FLAG) $(MANIFEST) $(PROG) $(SRC) $(OBJ)
	
$(OBJ): $(SRC)
	$(JC) $(JC_FLAG) $(SRC)

clean:
	rm -f $(OBJ)

tar:
	tar -czvf GUI.tar.gz ../gbad_gui/*.java ../gbad_gui/Makefile ../gbad_gui/manifest.txt ../gbad_gui/graph.g ../gbad_gui/mdl.g  ../gbad_gui/mps.g  ../gbad_gui/norm.g  ../gbad_gui/prob.g ../gbad_gui/bin/
