import java.util.StringTokenizer;

import java.io.BufferedReader;
import java.io.File;
import java.io.InputStreamReader;
import java.io.IOException;

import java.awt.Image;

import javax.swing.ImageIcon;
import javax.swing.JLabel;
import javax.swing.SwingConstants;

/*
This class is a JLabel that displays the number of instances of gbad
running in the background.  In order to actually update the count
displayed, a thread must be created and started with the instance of
this class as the runnable argumented.
*/
public class UpdateGBADCount extends JLabel implements Runnable
{
	// the gbad count is updated every UPDATE_DELAY milliseconds
	public static final int UPDATE_DELAY = 5000;
	
	// the name of the gbad executable to check for
	public static final String gbadUnixName = "gbad";
	public static final String gbadEXEName = "gbad.exe";
	
	private static String gbadCountDescription = "Number of GBAD processing running in the background.";
	
	// gbad icon
	public static final Image GBAD_IMAGE = (new ImageIcon(Images.GBAD_ICON, "GBAD")).getImage().getScaledInstance(75, 60, Image.SCALE_SMOOTH);
	
	// flag to use to stop excution of the run method
	protected boolean run;
	
	public UpdateGBADCount()
	{
		super("x 0", new ImageIcon(GBAD_IMAGE), SwingConstants.LEFT);
		setToolTipText(gbadCountDescription);
	}
	
	public void stop()
	{
		run = false;
	}
	
	/*
	Using the windows TaskList.exe program, the number of instances of gbad are counted and returned.
	*/
	private int getTaskListCount() throws IOException
	{
		int count = 0;
		File file;
		String line;
		String processName;
		StringTokenizer tokenizer;
		
		Process process;
		BufferedReader reader;
		
		process = Runtime.getRuntime().exec(System.getenv("windir") +"\\system32\\"+"tasklist.exe");
		
		reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
		
		line = reader.readLine();
		
		while(line != null)
		{
			tokenizer = new StringTokenizer(line);
			
			// the name of the program is the first token
			if(tokenizer.countTokens() >= 1)
			{
				processName = tokenizer.nextToken();
				
				file = new File(processName);
				if(file.getName().equals(gbadEXEName))
				{
					count++;
				}
			}
			
			line = reader.readLine();
		}
		reader.close();
		
		return count;
	}
	
	/*
	Using the unix ps program, the number of instances of gbad are counted and returned.
	*/
	private int getPSCount() throws IOException
	{
		int count = 0;
		File file;
		String line;
		String processName;
		StringTokenizer tokenizer;
		
		Process process;
		BufferedReader reader;
		
		process = Runtime.getRuntime().exec("ps -e");
		
		//Process p = Runtime.getRuntime().exec(System.getenv("windir") +"\\system32\\"+"tasklist.exe");
		
		reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
		
		line = reader.readLine();
		
		while(line != null)
		{
			tokenizer = new StringTokenizer(line);
			
			// the name of the program is the 4th token
			if(tokenizer.countTokens() >= 4)
			{
				tokenizer.nextToken();
				tokenizer.nextToken();
				tokenizer.nextToken();
				processName = tokenizer.nextToken();
				
				file = new File(processName);
				if(file.getName().equals(gbadUnixName))
				{
					count++;
				}
			}
			
			line = reader.readLine();
		}
		reader.close();
		
		return count;
	}
	
	private int getCount() throws IOException
	{
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
		
		
		try
		{
			// if the OS is windows use the TaskList program
			// for any other OS, try the ps program
			if(os.contains("Windows"))
			{
				return getTaskListCount();
			}
			else
			{
				return getPSCount();
			}
		}
		catch(Exception e)
		{
			return -1;
		}
	}
	
	public void run()
	{
		run = true;
		
		int count;
		
		while(run)
		{
			count = 0;
			
			try
			{
				count = getCount();
			}
			catch (IOException ioe)
			{
				count = -1;
			}
			
			if(count > -1)
			{
				setText("x " + count);
			}
			else
			{
				setText("x ???");
			}
			
			try
			{
				Thread.sleep(UPDATE_DELAY);
			}
			catch(InterruptedException ie)
			{
				/*
				unable to sleep
				if this loop can never sleep it will chew up CPU cycles
				stop the gbad count update if this can not sleep
				*/
				run = false;
			}
		}
	}
}
