import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

import java.awt.Color;
import java.awt.Font;

import javax.swing.BorderFactory;
import javax.swing.JFileChooser;
import javax.swing.JOptionPane;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;

import javax.swing.border.TitledBorder;

/*
This class is responsible for displaying the text results of a gbad run.
*/
public class GraphResults extends JScrollPane
{
	private static final String defaultFileText = "[No Results Selected]";
	
	private String graphFileName;
	private String dotFileName;
	private JTextArea textArea;
	private File lastDir;
	
	public GraphResults()
	{
		super();
		
		updateFileName(defaultFileText);
		
		graphFileName = null;
		dotFileName = null;
		
		textArea = new JTextArea();
		textArea.setEditable(false);
		textArea.setFont(new Font("Monospaced", Font.PLAIN, 12));
		
		setViewportView(textArea);
		
		try
		{
			String cwd = System.getProperty("user.dir");
			if(cwd == null)
			{
				throw new SecurityException();
			}
			
			lastDir = new File(cwd);
		}
		catch(SecurityException se)
		{
			lastDir = new File(".");
		}
	}
	
	/*
	Returns the name of the graph input file repoted in the results file.
	If no results file is currently opened or no graph file name was found
	when reading the results file, null is returned.
	*/
	public String getGraphFileName()
	{
		return graphFileName;
	}
	
	/*
	Returns the name of the dot output file repoted in the results file.
	If no results file is currently opened or no dot output file was found
	when reading the results file, null is returned.
	*/
	public String getDotFileName()
	{
		return dotFileName;
	}
	
	/*
	Clears the result display and resets file names.
	*/
	public void close()
	{
		graphFileName = null;
		dotFileName = null;
		textArea.setText("");
		
		updateFileName(defaultFileText);
	}
	
	/*
	Prompts the user for a results file to open.
	If the users selects a valid graph file, open(fileName) is called.
	*/
	public boolean open() throws FileNotFoundException, IOException
	{
		try
		{
			CustomFileFilter filter = new CustomFileFilter();
			filter.addExtension("result");
			filter.setDescription(".result (GBAD Result File)");
			
			JFileChooser chooser = new JFileChooser(lastDir);
			chooser.setFileFilter(filter);
			
			int returnVal = chooser.showOpenDialog(this);
			if(returnVal == JFileChooser.APPROVE_OPTION)
			{
				File inputFile = chooser.getSelectedFile();
				
				if(inputFile.getParentFile() != null)
				{
					lastDir = inputFile.getParentFile();
				}
				
				return open(inputFile);
			}
		}
		catch(Exception e)
		{
			JOptionPane.showMessageDialog(this, "Unable to access user files.", "Access Control Exception", JOptionPane.ERROR_MESSAGE);
		}
		
		return false;
	}
	
	/*
	Opens and displays inputFile.  If the graph input file and / or dot
	output file can be determined the values are set to graphFileName
	and dotFileName, respectively.  If the values can not be determined the
	corisponding variables are set to null.
	*/
	public boolean open(File inputFile) throws FileNotFoundException, IOException
	{
		// close the previous file first
		close();
		
		BufferedReader file = new BufferedReader(new FileReader(inputFile));
		
		StringBuffer content = new StringBuffer();
		String line = file.readLine();
		
		while(line != null)
		{
			content.append(line + "\n");
			
			if(graphFileName == null)
			{
				// if input file was found
				if(line.indexOf("Input file") != -1)
				{
					int index = line.indexOf(". ");
					graphFileName = line.substring(index+1).trim();
				}	
			}
			
			if(dotFileName == null)
			{
				if(line.indexOf("Dot file") != -1)
				{
					int index = line.lastIndexOf(". ");
					dotFileName = line.substring(index+1).trim();
				}	
			}
			
			line = file.readLine();
		}
		
		file.close();
		
		textArea.setText(content.toString());
		
		textArea.setCaretPosition(0);
		
		updateFileName(inputFile.toString());
		
		return true;
	}
	
	// cheange the display name to fileName
	private void updateFileName(String fileName)
	{
		setBorder(BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.blue, 0), fileName, TitledBorder.RIGHT, TitledBorder.TOP, new Font("Comic Sans MS", Font.BOLD, 15), Color.red));
	}
}
