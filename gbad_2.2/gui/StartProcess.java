import java.util.Vector;
import java.util.concurrent.Semaphore;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.InputStreamReader;
import java.io.IOException;
import java.io.PrintWriter;

import java.awt.Dimension;
import java.awt.Font;

import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;

import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.UIManager;
import javax.swing.UnsupportedLookAndFeelException;

/*
This class can be used to start an external process and capture
the process's output.

If the OS is not OSX, then a window will open to display the program output as well.

If the OS is OSX, then no window will be displayed, since any GUI application in OSX
must have a doc icon and therefor can not run in the background after the user logs off,
or closes the doc icon.
*/
public class StartProcess implements WindowListener
{
	public static final int OK_CODE = 0;
	public static final int OUTPUT_CREATE_ERROR = 1;
	public static final int START_PROCESS_ERROR = 2;
	public static final int IO_ERROR = 3;
	
	protected File file;
	protected File directory;

	protected JTextArea textArea;
	
	protected Vector<String> args;
	
	protected boolean running;
	
	public static final String CLASS_PATH = StartProcess.class.getProtectionDomain().getCodeSource().getLocation().getPath();
	public static final String CLASS_NAME = StartProcess.class.getName().toString();
	
	protected JFrame frame;
	
	protected Semaphore lock;
	
	protected int returnCode;
	
	/*
	Use args to start up an external process.  Anything on stdout or stderr is
	written to fileName.
	*/
	public StartProcess(String fileName, Vector<String> args)
	{
		if(args == null)
		{
			throw new NullPointerException("args parameter can not be null.");
		}
		
		String os;
		
		try
		{
			os = System.getProperty("os.name");
			if(os == null)
			{
				throw new SecurityException();
			}
		}
		catch(SecurityException se)
		{
			os = "Windows";
		}
		
		lock = new Semaphore(1);
		
		if(!os.equals("Mac OS X") || true)
		{
			try
			{
				lock.acquire();
			}
			catch(InterruptedException ie)
			{
				// ignore
			}
			
			frame = new JFrame("GBAD Output");
			frame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
			frame.setSize(800,600);
			
			frame.addWindowListener(this);
			
			Dimension screenSize = frame.getToolkit().getScreenSize();
			int screenWidth = screenSize.width;
			int screenHeight = screenSize.height;
			
			int xLoc = (screenWidth/3) - (frame.getWidth()/3);
			int yLoc = (screenHeight/3) - (frame.getHeight()/3);
			
			frame.setLocation(xLoc, yLoc);
						
			setupLookAndFeel(os);
	
			textArea = new JTextArea();
			textArea.setEditable(false);
			textArea.setFont(new Font("Monospaced", Font.PLAIN, 12));
			
			frame.add(new JScrollPane(textArea));
			
			frame.setVisible(true);
		}
			
		file = new File(fileName);
		
		// check if their is a parent directory
		if(file.getParent() == null)
		{
			directory = new File(".");
		}
		else
		{
			// make any directories needed to write the output
			directory = new File(file.getParent());
			
			try
			{
				directory.mkdirs();
			}
			catch(SecurityException se)
			{
				// ignore 
			}
		}
		
		this.args = args;
		running = false;
		returnCode = OK_CODE;
	}
	
	public int getReturnCode()
	{
		return returnCode;
	}
	
	// Sets the look and feel to the systems look and feel.
	private void setupLookAndFeel(String os)
	{
		try
		{
			if(os.equals("Mac OS X"))
			{
				System.setProperty("apple.laf.useScreenMenuBar", "true");
				//System.setProperty("com.apple.mrj.application.growbox.intrudes", "false");
			}
			else
			{
				System.clearProperty("apple.laf.useScreenMenuBar");
			}
		}
		catch(Exception e)
		{
			// ignore
		}
		
		try
		{
			UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
			
			//UIManager.setLookAndFeel("com.sun.java.swing.plaf.motif.MotifLookAndFeel");
		}
		catch(InstantiationException ie)
		{
			//JOptionPane.showMessageDialog(frame, ie.getMessage(), "Error Setting Look And Feel", JOptionPane.ERROR_MESSAGE);
		}
		catch(IllegalAccessException iae)
		{
			//JOptionPane.showMessageDialog(frame, iae.getMessage(), "Error Setting Look And Feel", JOptionPane.ERROR_MESSAGE);
		}
		catch(UnsupportedLookAndFeelException ulafe)
		{
			//JOptionPane.showMessageDialog(frame, ulafe.getMessage(), "Error Setting Look And Feel", JOptionPane.ERROR_MESSAGE);
		}
		catch(ClassNotFoundException cnfe)
		{
			//JOptionPane.showMessageDialog(frame, cnfe.getMessage(), "Error Setting Look And Feel", JOptionPane.ERROR_MESSAGE);
		}
	}
	
	/*
	This function will start an instance of this classes main up as an external
	program.  The main method will simply create an instance of this class using
	fileName and args, and call the run method.
	*/
	public static Process start(String fileName, Vector<String> args) throws IOException
	{	
		int index = 0;
		args.add(index++, "java");
		args.add(index++, "-cp");
		args.add(index++, StartProcess.CLASS_PATH);
		args.add(index++, StartProcess.CLASS_NAME);
		args.add(index++, fileName);
		
		System.out.println(getCommandLine(new File(fileName), args));
				
		ProcessBuilder processBuilder = new ProcessBuilder(args);
		
		Process process = processBuilder.start();
		
		return process;
	}
	
	public void run()
	{
		synchronized(this)
		{
			running = true;
		}
		
		System.out.println(toString() + "\n\n");
		
		if(frame != null)
		{
			textArea.append(toString() + "\n\n");
		}
		
		try
		{
			PrintWriter writer = new PrintWriter(new BufferedWriter(new FileWriter(file)));
			
			ProcessBuilder processBuilder = new ProcessBuilder(args);
			processBuilder.directory(directory);
			
			// write stderr to stdout
			processBuilder.redirectErrorStream(true);
		
			try
			{
				Process process = processBuilder.start();
				
				try
				{
					BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
					
					String line = reader.readLine();
					
					// keep reading from the program and displing/storing the output
					while(line != null)
					{
						if(frame != null)
						{
							textArea.append(line + "\n");
						}
						
						writer.println(line);
						writer.flush();
						line = reader.readLine();
					}
					
					writer.flush();
					writer.close();
				}
				catch(Exception e)
				{
					returnCode = IO_ERROR;
					
					if(frame != null)
					{
						textArea.append(e.getMessage() + "\n");
					}
				}
			}
			catch(Exception e)
			{
				returnCode = START_PROCESS_ERROR;
				
				if(frame != null)
				{
					textArea.append(e.getMessage() + "\n");
				}
			}	
		}
		catch(IOException e)
		{
			returnCode = OUTPUT_CREATE_ERROR;
			
			if(frame != null)
			{
				textArea.append(e.getMessage() + "\n");
			}
		}
		
		try
		{
			lock.acquire();
		}
		catch(InterruptedException ie)
		{
			// ignore
		}
		
		synchronized(this)
		{
			running = false;
		}
	}
	
	// returns a string that is similar to what would be typed in a shell to start the process
	public static String getCommandLine(File file, Vector<String> args)
	{
		String str = "";
		
		for(int ii=0; ii < args.size(); ii++)
		{
			str += args.elementAt(ii) + " ";
		}
		
		str += " > " + file.toString();
		
		return str;
	}
	
	public String toString()
	{
		return getCommandLine(file, args);
	}
	
	public void windowClosing(WindowEvent e)
	{
		lock.release();
	}

	public void windowClosed(WindowEvent e)
	{
		
	}

	public void windowOpened(WindowEvent e)
	{
		
	}

	public void windowIconified(WindowEvent e)
	{
		
	}

	public void windowDeiconified(WindowEvent e)
	{
		
	}

	public void windowActivated(WindowEvent e)
	{
		
	}

	public void windowDeactivated(WindowEvent e)
	{
		
	}

	public void windowGainedFocus(WindowEvent e)
	{
		
	}

	public void windowLostFocus(WindowEvent e)
	{
		
	}

	public void windowStateChanged(WindowEvent e)
	{
		
	}
		
	public static void main(String[] args)
	{
		if(args.length < 2)
		{
			System.err.println("too few arguments provided");
			
			System.exit(StartProcess.OUTPUT_CREATE_ERROR);
		}
		else
		{
			Vector<String> runArgs = new Vector<String>();
			
			for(int i=1; i<args.length; i++)
			{
				runArgs.add(args[i]);
			}
			
			try
			{
				StartProcess newProgram = new StartProcess(args[0], runArgs);
				newProgram.run();
				
				System.exit(newProgram.getReturnCode());
			}
			catch(Exception e)
			{
				e.printStackTrace();
				System.exit(START_PROCESS_ERROR);
			}
		}
	}
}
