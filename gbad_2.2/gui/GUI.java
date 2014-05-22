import java.util.*;

import java.io.*;

import javax.swing.*;
import javax.swing.border.*;

import java.awt.*;
import java.awt.event.*;

import java.text.*;

public class GUI extends JFrame implements ActionListener, WindowListener
{
	public static final String DATE_FORMAT_NOW = "yyyy-MM-dd_HH.mm.ss";
	
	//ClassLoader cl = this.getClass().getClassLoader();
	//cl.getResource("images/save.gif")
	
	protected Parameters parameters;
	protected GraphDisplay graphDisplay;
	
	protected GBADPreferences preferences;
	
	protected GBADMenu gbadMenu;
	
	public GUI()
	{
		super("Graph Based Anomaly Detection");
		setDefaultCloseOperation(DO_NOTHING_ON_CLOSE);
		addWindowListener(this);
		
		Dimension screenSize = getToolkit().getScreenSize();
		int screenWidth = screenSize.width;
		int screenHeight = screenSize.height;
		
		int width = 800;
		int height = 650;
		
		if(width > screenWidth-100)
		{
			width = screenWidth-100;
		}
		
		if(height > screenHeight-100)
		{
			height = screenHeight-100;
		}
		
		setSize(width, height);
		
		int xLoc = (screenWidth/2) - (getWidth()/2);
		int yLoc = (screenHeight/2) - (getHeight()/2);
		
		setLocation(xLoc, yLoc);
		
		setupLookAndFeel();
		
		preferences = new GBADPreferences(this);
		
		System.out.println("addMenuBar");
		addMenuBar();
		System.out.println("addContent");
		addContent();
		
		setIconImage((new ImageIcon(Images.GBAD_ICON, "GBAD")).getImage().getScaledInstance(50, 50, Image.SCALE_SMOOTH));
	}
	
	public GUI(String fileName)
	{
		this();
		
		System.out.println(fileName);
		
		String[] tokens = fileName.split("\\.");
		
		if(tokens.length > 0)
		{
			if(tokens[tokens.length-1].equals("g"))
			{
				graphDisplay.openGraph(fileName);
			}
			else if(tokens[tokens.length-1].equals("result"))
			{
				graphDisplay.openResults(fileName);
			}
		}
	}
	
	protected void addMenuBar()
	{
		gbadMenu = new GBADMenu();
		gbadMenu.addActionListener(this);
		setJMenuBar(gbadMenu);
	}
	
	protected void addContent()
	{
		JPanel content = new JPanel();
		content.setLayout(new BorderLayout());
		
		parameters = new Parameters();
		parameters.addActionListener(this);
		content.add(parameters, BorderLayout.WEST);
		
		graphDisplay = new GraphDisplay();
		content.add(graphDisplay, BorderLayout.CENTER);
		setContentPane(content);
	}
	
	protected void setupLookAndFeel()
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
		
		if(os.equals("Mac OS X"))
		{
			System.setProperty("apple.laf.useScreenMenuBar", "true");
			//System.setProperty("com.apple.mrj.application.growbox.intrudes", "false");
		}
		else
		{
			System.clearProperty("apple.laf.useScreenMenuBar");
		}
		
		try
		{
			UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
			
			//UIManager.setLookAndFeel("com.sun.java.swing.plaf.motif.MotifLookAndFeel");
		}
		catch(InstantiationException ie)
		{
			//JOptionPane.showMessageDialog(this, ie.getMessage(), "Error Setting Look And Feel", JOptionPane.ERROR_MESSAGE);
		}
		catch(IllegalAccessException iae)
		{
			//JOptionPane.showMessageDialog(this, iae.getMessage(), "Error Setting Look And Feel", JOptionPane.ERROR_MESSAGE);
		}
		catch(UnsupportedLookAndFeelException ulafe)
		{
			//JOptionPane.showMessageDialog(this, ulafe.getMessage(), "Error Setting Look And Feel", JOptionPane.ERROR_MESSAGE);
		}
		catch(ClassNotFoundException cnfe)
		{
			//JOptionPane.showMessageDialog(this, cnfe.getMessage(), "Error Setting Look And Feel", JOptionPane.ERROR_MESSAGE);
		}
	}
	
	public static String getCurTime()
	{
		Calendar cal = Calendar.getInstance();
		SimpleDateFormat sdf = new SimpleDateFormat(DATE_FORMAT_NOW);
		return sdf.format(cal.getTime());
	}
	
	protected void startGBAD()
	{
		try
		{
			if(!graphDisplay.askToSaveGraph("Do you want to save the changes to " + graphDisplay.getGraphFileName() + " before running GBAD?"))
			{
				return;
			}
			
			File inputFile = graphDisplay.getGraphFile();
			
			if((inputFile == null) || (!inputFile.exists()))
			{
				JOptionPane.showMessageDialog(this, "Unable to locate file " + graphDisplay.getGraphFileName() + ".", "Error Locating File", JOptionPane.ERROR_MESSAGE);
				return;
			}
			
			Vector<String> args = parameters.getParameters();
			
			String outputName;
			String dotName;
			String anomString;
			
			if(inputFile.getParent() == null)
			{
				outputName = "./";
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
				outputName = inputFile.getParent() + "/";
				dotName = inputFile.getParent() + "/dot/";
				
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
			
			outputName += inputFile.getName();
			dotName += inputFile.getName();
			
			String anomAlg = parameters.getAnomalyAlgorithm();
			
			if(anomAlg.equals("None"))
			{
				anomString = ".subdue";
			}
			else if(anomAlg.equals("-mdl"))
			{
				anomString = ".mdl";
			}
			else if(anomAlg.equals("-mps"))
			{
				anomString = ".mps";
			}
			else if(anomAlg.equals("-prob"))
			{
				anomString = ".prob";
			}
			else
			{
				anomString = "__" + anomAlg + "__";
			}
			
			outputName += anomString;
			dotName += anomString;
			
			String curTime = getCurTime();
			
			outputName += "_" + curTime;
			dotName += "_" + curTime;
			
			outputName += ".result";
			dotName += ".dot";
			
			args.add("-dot");
			args.add(dotName);
			args.add(inputFile.toString());
			
			System.out.println(StartProcess.getCommandLine(new File(outputName), args));
			
			try
			{
				Process process = StartProcess.start(outputName, args);
				
				Thread thread = new Thread(new GBADProcessMonitor(outputName, process, this));
				thread.start();
			}
			catch(IOException ioe)
			{
				JOptionPane.showMessageDialog(this, "Unable to start GBAD process.", "Error Starting GBAD", JOptionPane.ERROR_MESSAGE);
			}
		}
		catch(NumberFormatException nfe)
		{
			JOptionPane.showMessageDialog(this, nfe.getMessage(), "Invalid Parameter Value", JOptionPane.ERROR_MESSAGE);
		}
	}
	
	protected void gbadComplete(String outputName)
	{
		int option = JOptionPane.showConfirmDialog(this, "GBAD process exited.  Do you want to open results?", "GBAD Process", JOptionPane.YES_NO_OPTION, JOptionPane.WARNING_MESSAGE);
		
		if(option == JOptionPane.YES_OPTION)
		{
			graphDisplay.openResults(outputName);
		}
	}
	
	protected void gbadError(String error)
	{
		int option = JOptionPane.showConfirmDialog(this, error, "GBAD Process", JOptionPane.DEFAULT_OPTION, JOptionPane.WARNING_MESSAGE);
	}
	
	public void actionPerformed(ActionEvent e)
	{
		if(e.getSource() == gbadMenu)
		{
			if(e.getID() == gbadMenu.NEW_COMMAND)
			{
				graphDisplay.newGraph();
			}
			else if(e.getID() == gbadMenu.OPEN_GRAPH_COMMAND)
			{
				graphDisplay.openGraph();
			}
			else if(e.getID() == gbadMenu.OPEN_RESULTS_COMMAND)
			{
				graphDisplay.openResults();
			}
			else if(e.getID() == gbadMenu.CLOSE_COMMAND)
			{
				WindowEvent newEvent = new WindowEvent(this, 0);
				this.windowClosing(newEvent); 
			}
			else if(e.getID() == gbadMenu.SAVE_COMMAND)
			{
				graphDisplay.saveGraph();
			}
			else if(e.getID() == gbadMenu.SAVE_AS_COMMAND)
			{
				graphDisplay.saveAsGraph();
			}
			else if(e.getID() == gbadMenu.PREFERENCES_COMMAND)
			{
				preferences.pack();
				preferences.setSize(preferences.getPreferredSize());
				Point p = getLocationOnScreen();
				preferences.setLocation((int)p.getX()+10, (int)p.getY()+10);
				preferences.setVisible(true);
			}
		}
		else if(e.getSource() == parameters)
		{
			if(e.getActionCommand().equals(parameters.startGBADText))
			{
				startGBAD();
			}
		}
	}
	
	public void windowClosing(WindowEvent e)
	{
		if(graphDisplay.askToSaveGraph("Do you want to save the changes to " + graphDisplay.getGraphFileName() + " before closing?"))
		{
			System.exit(0);	
		}
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
}

class GBADProcessMonitor implements Runnable
{
	String fileName;
	Process process;
	GUI listener;
	
	public GBADProcessMonitor(String fileName, Process process, GUI listener)
	{
		this.process = process;
		this.fileName = fileName;
		this.listener = listener;
	}
	
	public void run()
	{
		try
		{
			int returnValue = process.waitFor();
			System.out.println("VALUE OF " + returnValue);
			
			if(returnValue != 0)
			{
				String error = "An unexpected error occurred when staring gbad.  ";
				
				if(returnValue == StartProcess.OUTPUT_CREATE_ERROR)
				{
					error = "Unable to create output file (" + fileName + ").";
				}
				else if(returnValue == StartProcess.START_PROCESS_ERROR)
				{
					error = "Unable to start gbad process.";
				}
				else if(returnValue == StartProcess.IO_ERROR)
				{
					error = "A problem occurred when attempting to read/write gbad output.";
				}
				
				listener.gbadError(error);
			}
			else
			{
				listener.gbadComplete(fileName);
			}
		}
		catch(InterruptedException ie)
		{
		
		}
	}
}