import java.io.*;

import java.util.*;

import javax.swing.*;
import javax.swing.border.*;

import java.awt.*;
import java.awt.event.*;

public class GraphDisplay extends JTabbedPane
{
	GraphText graphText;
	GraphResults graphResults;
	GraphVisualization graphVisualization;
	
	public static final String dataDir = "./graphs/";
	
	public GraphDisplay()
	{
		super();
		
		graphText = new GraphText();
		graphResults = new GraphResults();
		graphVisualization = new GraphVisualization();
		
		add("Visualization", graphVisualization);
		add("Results", graphResults);
		add("Input File", graphText);
		
		setEnabledAt(0, false);
		setEnabledAt(1, false);
		setEnabledAt(2, false);
	}
	
	public String getGraphFileName()
	{
		return graphText.getFileName();
	}
	
	public File getGraphFile()
	{
		return graphText.getFile();
	}
	
	public boolean hasChangedGraph()
	{
		return graphText.hasChanged();
	}
	
	public boolean newGraph()
	{
		boolean result = false;
		
		try
		{
			result = graphText.newGraph();
			
			if(result)
			{
				setEnabledAt(indexOfComponent(graphText), true);
				setSelectedIndex(indexOfComponent(graphText));
				
				closeResults();
				closeVisualization();
			}
		}
		catch(IOException ioe)
		{
			JOptionPane.showMessageDialog(this, ioe.getMessage(), "Error Saving File", JOptionPane.ERROR_MESSAGE);
		}
		
		return result;
	}
	
	public boolean saveAsGraph()
	{
		boolean result = false;
		
		try
		{
			result = graphText.saveAs();
			
			if(result)
			{
				setEnabledAt(indexOfComponent(graphText), true);
				
				closeResults();
				closeVisualization();
				
				try
				{	
					String dotName = createDotFile(graphText.getFile().toString());
					graphVisualization.open(dotName);
					
					setEnabledAt(indexOfComponent(graphVisualization), true);
					setSelectedIndex(indexOfComponent(graphVisualization));
				}
				catch(IOException ioe)
				{
					JOptionPane.showMessageDialog(this, ioe.getMessage(), "Error Running Dot.", JOptionPane.ERROR_MESSAGE);
					setSelectedIndex(indexOfComponent(graphText));
				}
			}
		}
		catch(IOException ioe)
		{
			JOptionPane.showMessageDialog(this, ioe.getMessage(), "Error Saving File", JOptionPane.ERROR_MESSAGE);
		}
		
		return result;
	}
	
	public boolean  saveGraph()
	{
		boolean result = false;
		
		try
		{
			result = graphText.save();
			
			if(result)
			{
				setEnabledAt(indexOfComponent(graphText), true);
				
				closeResults();
				closeVisualization();
				
				try
				{	
					String dotName = createDotFile(graphText.getFile().toString());
					graphVisualization.open(dotName);
					
					setEnabledAt(indexOfComponent(graphVisualization), true);
					setSelectedIndex(indexOfComponent(graphVisualization));
				}
				catch(IOException ioe)
				{
					JOptionPane.showMessageDialog(this, ioe.getMessage(), "Error Running Dot.", JOptionPane.ERROR_MESSAGE);
					setSelectedIndex(indexOfComponent(graphText));
				}
			}
		}
		catch(IOException ioe)
		{
			JOptionPane.showMessageDialog(this, ioe.getMessage(), "Error Saving File", JOptionPane.ERROR_MESSAGE);
		}
		
		return result;
	}
	
	public boolean askToSaveGraph(String question)
	{
		boolean result = false;
		
		try
		{
			result = graphText.askToSave(question);
		}
		catch(IOException ioe)
		{
			JOptionPane.showMessageDialog(this, ioe.getMessage(), "Error Saving File", JOptionPane.ERROR_MESSAGE);
		}
		return result;
	}
	
	public void openGraphHelper()
	{
		closeResults();
		closeVisualization();
		
		setEnabledAt(indexOfComponent(graphText), true);
		
		try
		{	
			String dotName = createDotFile(graphText.getFile().toString());
			graphVisualization.open(dotName);
			
			setEnabledAt(indexOfComponent(graphVisualization), true);
			setSelectedIndex(indexOfComponent(graphVisualization));
		}
		catch(IOException ioe)
		{
			JOptionPane.showMessageDialog(this, ioe.getMessage(), "Error Running Dot.", JOptionPane.ERROR_MESSAGE);
			setSelectedIndex(indexOfComponent(graphText));
		}
	}
	
	public void openGraph()
	{
		if(askToSaveGraph("Do you want to save the changes to " + graphText.getFileName() + " before opening a new graph?"))
		{
			try
			{
				if(graphText.open())
				{
					openGraphHelper();
				}
			}
			catch(FileNotFoundException fnfe)
			{
				JOptionPane.showMessageDialog(this, fnfe.getMessage(), "Error Opening File", JOptionPane.ERROR_MESSAGE);
			}
			catch(IOException ioe)
			{
				JOptionPane.showMessageDialog(this, ioe.getMessage(), "Error Opening File", JOptionPane.ERROR_MESSAGE);
			}
		}
	}
	
	public void openGraph(String fileName)
	{
		if(askToSaveGraph("Do you want to save the changes to " + graphText.getFileName() + " before opening a new graph?"))
		{
			try
			{
				if(graphText.open(new File(fileName)))
				{
					openGraphHelper();
				}			
			}
			catch(FileNotFoundException fnfe)
			{
				JOptionPane.showMessageDialog(this, fnfe.getMessage(), "Error Opening File", JOptionPane.ERROR_MESSAGE);
			}
			catch(IOException ioe)
			{
				JOptionPane.showMessageDialog(this, ioe.getMessage(), "Error Opening File", JOptionPane.ERROR_MESSAGE);
			}
		}
	}
	
	private void openResultsHelper()
	{
		closeText();
		closeVisualization();
		
		setSelectedIndex(indexOfComponent(graphResults));
		setEnabledAt(indexOfComponent(graphResults), true);
		
		String error = "";
		
		String graphFile = graphResults.getGraphFileName();
		String dotFile = graphResults.getDotFileName();
		
		if(dotFile != null)
		{
			graphVisualization.open(graphResults.getDotFileName());
			
			setEnabledAt(indexOfComponent(graphVisualization), true);
		}
		else
		{
			error += "Unable to determine the name of the dot output file.  ";
		}
		
		if(graphFile != null)
		{
			System.out.println("Try to open " + graphFile);
			
			try
			{
				graphText.open(new File(graphFile));
				
				setEnabledAt(indexOfComponent(graphText), true);
			}
			catch(Exception e)
			{
				error += "Unable to open graph input file (" + e.getMessage() + ").  ";	
			}
		}
		else
		{
			error += "Unable to determine the name of the graph input file.  ";
		}
		
		if(!error.equals(""))
		{
			JOptionPane.showMessageDialog(this, error, "Error Reading Results.", JOptionPane.ERROR_MESSAGE);
		}
	}
	
	public void openResults()
	{
		if(askToSaveGraph("Do you want to save the changes to " + graphText.getFileName() + " before opening results?"))
		{
			try
			{
				if(graphResults.open())
				{
					openResultsHelper();
				}
			}
			catch(FileNotFoundException fnfe)
			{
				JOptionPane.showMessageDialog(this, fnfe.getMessage(), "Error Opening Results File", JOptionPane.ERROR_MESSAGE);
			}
			catch(IOException ioe)
			{
				JOptionPane.showMessageDialog(this, ioe.getMessage(), "Error Opening Results File", JOptionPane.ERROR_MESSAGE);
			}
		}
	}
	
	public void openResults(String resultFile)
	{
		if(askToSaveGraph("Do you want to save the changes to " + graphText.getFileName() + " before opening results?"))
		{
			try
			{
				if(graphResults.open(new File(resultFile)))
				{
					openResultsHelper();
				}
			}
			catch(FileNotFoundException fnfe)
			{
				JOptionPane.showMessageDialog(this, fnfe.getMessage(), "Error Opening Results File", JOptionPane.ERROR_MESSAGE);
			}
			catch(IOException ioe)
			{
				JOptionPane.showMessageDialog(this, ioe.getMessage(), "Error Opening Results File", JOptionPane.ERROR_MESSAGE);
			}
		}
	}
	
	public void closeResults()
	{
		graphResults.close();
		setEnabledAt(indexOfComponent(graphResults), false);
	}
	
	public void closeVisualization()
	{
		graphVisualization.close();
		setEnabledAt(indexOfComponent(graphVisualization), false);
	}
	
	public void closeText()
	{
		graphText.close();
		setEnabledAt(indexOfComponent(graphText), false);
	}
	
	private String createDotFile(String graphFileName) throws IOException
	{
		Vector<String> cmd = new Vector<String>();
		
		String graph2dot;
		
		try
		{
			graph2dot = System.getProperty(GBADPreferences.GRAPH2DOT);
			if(graph2dot == null)
			{
				throw new SecurityException();
			}
		}
		catch(SecurityException se)
		{
			graph2dot = GBADPreferences.GRAPH2DOT; 
		}
		
		cmd.add(graph2dot);
		
		cmd.add(graphFileName);
		
		String dotName;
		
		File graphFile = new File(graphFileName);
		if(graphFile.getParent() == null)
		{
			dotName = "./dot/";
			
			try
			{
				(new File(dotName)).mkdirs();
			}
			catch(Exception ex)
			{
				// can not create directory, ignore
				ex.printStackTrace();
			}
		}
		else
		{
			dotName = graphFile.getParent() + "/dot/";
			
			try
			{
				(new File(dotName)).mkdirs();
			}
			catch(Exception ex)
			{
				// can not create directory, ignore
				ex.printStackTrace();
			}
		}
		
		dotName += graphFile.getName();
		
		dotName += "_" + GUI.getCurTime() + ".dot";
		
		cmd.add(dotName);
		
		// run the conversion program
		ProcessBuilder processBuilder = new ProcessBuilder(cmd);
		Process process;
		
		StringBuffer error = new StringBuffer();
		
		try
		{
			process = processBuilder.start();
			process.waitFor();
			
			BufferedReader reader = new BufferedReader(new InputStreamReader(process.getErrorStream()));
			
			String line = reader.readLine();
			
			while(line != null)
			{
				error.append(line);
				error.append("\n");
				line = reader.readLine();
			}
		}
		catch(IOException ioe)
		{
			throw new IOException("graph2dot: " + ioe.getMessage());
		}
		catch(InterruptedException ie)
		{
			ie.printStackTrace();
		}
		
		if(!(error.toString()).equals(""))
		{
			throw new IOException(error.toString());
		}
		
		return dotName;
	}
}