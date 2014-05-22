import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;

import javax.swing.BorderFactory;
import javax.swing.JFileChooser;
import javax.swing.JOptionPane;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;

import javax.swing.border.TitledBorder;

import java.awt.Color;
import java.awt.Font;

/*
This class is used to display a graph (.g) file.
*/
public class GraphText extends JScrollPane
{
	// string that is displayed when no file is current opened
	private static final String defaultFileText = "[No Graph File Opened]";
	
	/*
	the file associated with the current content
	this can be null if not file is associated with the content
	*/
	private File graphFile;
	
	// the original text for the file
	private StringBuffer originalText;
	
	// text area to display the content
	private JTextArea textArea;
	
	// the directory the last open file was in
	// this is used to reopen the file chooser in that directory
	private File lastDir;
	
	public GraphText()
	{
		super();
		
		updateFileName(defaultFileText);
		
		graphFile = null;
		
		originalText = new StringBuffer();
		
		textArea = new JTextArea();
		textArea.setFont(new Font("Monospaced", Font.PLAIN, 12));
		textArea.setEditable(false);
		
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
		catch(SecurityException e)
		{
			lastDir = new File(".");
		}
	}
	
	/*
	Returns the current file name being displayed.
	*/
	public String getFileName()
	{
		if(graphFile == null)
		{
			return defaultFileText;
		}
		else
		{
			return graphFile.getName();
		}
	}
	
	/*
	Returns a File object of the current file being displayed.
	*/
	public File getFile()
	{
		// return a cop of graphFile
		File file = null;
		
		if(graphFile != null)
		{
			file = new File(graphFile.toString());
		}
		
		return file;
	}
	
	/*
	Returns true if the current content if the display does not equal
	the content store in the file.
	*/	
	public boolean hasChanged()
	{
		return !textArea.getText().equals(originalText.toString());
	}
	
	/*
	Clear the current graph and allow for a new one to be created.
	If hasChanged returns true, the user is first asked if they want to save
	the current content.
	*/
	public boolean newGraph() throws IOException
	{
		// if the content has changed, prompt the user to save
		if(hasChanged())
		{
			// ask the user to save
			int option = JOptionPane.showConfirmDialog(this, "Do you want to save the changes to " + getFileName() + " before closing?", "Save?", JOptionPane.YES_NO_CANCEL_OPTION, JOptionPane.WARNING_MESSAGE);
			
			// if the user wants to save
			if(option == JOptionPane.YES_OPTION)
			{
				// if the save was canceled
				if(!save())
				{
					return false;
				}
			}
			// if the save was canceled
			else if(option == JOptionPane.CANCEL_OPTION)
			{
				return false;
			}
		}
		
		// create the new blank content
		updateFileName(defaultFileText);
		graphFile = null;
		originalText = new StringBuffer();
		
		textArea.setText("");
		
		return true;
	}
	
	/*
	Will prompt the user for a file name and save the current graph in
	the specified file.
	*/
	public boolean saveAs() throws IOException
	{
		// use a custom filter to only select .g files
		CustomFileFilter filter = new CustomFileFilter();
		filter.addExtension("g");
		filter.setDescription(".g (Graph File)");
		
		try
		{
			JFileChooser chooser = new JFileChooser(lastDir);
			chooser.setSelectedFile(new File("graph.g"));
			chooser.setFileFilter(filter);
			
			int returnVal = chooser.showSaveDialog(this);
			
			// if the user picked a file to save to
			if(returnVal == JFileChooser.APPROVE_OPTION)
			{	
				// try to save the file
				File tempFile = chooser.getSelectedFile();
				
				if(tempFile.getParentFile() != null)
				{
					lastDir = tempFile.getParentFile();
				}
				
				PrintWriter pw = new PrintWriter(new BufferedWriter(new FileWriter(tempFile)));
				pw.print(textArea.getText());
				pw.flush();
				pw.close();
				
				originalText = new StringBuffer(textArea.getText());
				graphFile = tempFile;
				updateFileName(graphFile.toString());
				
				return true;
			}
		}
		catch(Exception e)
		{
			JOptionPane.showMessageDialog(this, "Unable to access user files.", "Access Control Exception", JOptionPane.ERROR_MESSAGE);
		}
		
		return false;
	}
	
	/*
	Save the current graph in the associated file.  If no file is associated
	with the content, the user is prompted for a file name.
	*/
	public boolean save() throws IOException
	{
		// if the graphFile is null use saveAs as no file is associated yet
		if(graphFile == null)
		{
			return saveAs();
		}
		else
		{
			// if the file has not changed their is no reason to save
			if(hasChanged())
			{
				PrintWriter pw = new PrintWriter(new BufferedWriter(new FileWriter(graphFile)));
				pw.print(textArea.getText());
				pw.flush();
				pw.close();
				
				System.out.println("saveing to " + graphFile);
				
				originalText = new StringBuffer(textArea.getText());
				
				return true;
			}
			else
			{
				System.out.println("no change");
				return true;
			}
		}
	}
	
	/*
	If hasChanged returns true, ask the user if they want to save the file.
	If the user selects yes, the save method will be called.  Returns true
	if the current content is/was stored in an associated file.  Otherwise
	returns false.
	*/
	public boolean askToSave(String question) throws IOException
	{
		// if the file has changed save it
		if(hasChanged())
		{
			int option = JOptionPane.showConfirmDialog(this, question, "Save?", JOptionPane.YES_NO_CANCEL_OPTION, JOptionPane.WARNING_MESSAGE);
			
			if(option == JOptionPane.YES_OPTION)
			{
				if(!save())
				{
					return false;
				}
			}
			else if(option == JOptionPane.CANCEL_OPTION)
			{
				return false;
			}
		}
		
		return true;
	}
	
	/*
	Prompted the user for a file to open and then call the open(file) function
	with the selected file.  Returns true if the file was opened, otherwise
	returns false.
	*/
	public boolean open() throws FileNotFoundException, IOException
	{
		CustomFileFilter filter = new CustomFileFilter();
		filter.addExtension("g");
		filter.setDescription(".g (GBAD Result File)");
		
		try
		{
			JFileChooser chooser = new JFileChooser(lastDir);
			chooser.setFileFilter(filter);
			
			int returnVal = chooser.showOpenDialog(this);
			if(returnVal == JFileChooser.APPROVE_OPTION)
			{
				File tempFile = chooser.getSelectedFile();
				
				if(tempFile.getParentFile() != null)
				{
					lastDir = tempFile.getParentFile();
				}
				
				return open(tempFile);
			}
		}
		catch(Exception e)
		{
			JOptionPane.showMessageDialog(this, "Unable to access user files.", "Access Control Exception", JOptionPane.ERROR_MESSAGE);
		}
		
		return false;
	}
	
	/*
	Opens and displays tempFile.
	*/
	public boolean open(File tempFile) throws FileNotFoundException, IOException
	{
		BufferedReader reader = new BufferedReader(new FileReader(tempFile));
		
		originalText = new StringBuffer();
		String line = reader.readLine();
		
		while(line != null)
		{
			originalText.append(line + "\n");
			line = reader.readLine();
		}
		
		reader.close();
		
		textArea.setText(originalText.toString());
		
		textArea.setCaretPosition(0);
		
		graphFile = tempFile;
		updateFileName(graphFile.toString());
		
		return true;
	}
	
	public void close()
	{
		graphFile = null;
		originalText = new StringBuffer();
		textArea.setText("");
		
		updateFileName(defaultFileText);
	}
	
	/*
	Changes the name displayed in the border to fileName.
	*/
	private void updateFileName(String fileName)
	{
		setBorder(BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.blue, 0), fileName, TitledBorder.RIGHT, TitledBorder.TOP, new Font("Comic Sans MS", Font.BOLD, 15), Color.red));
	}
}
