import java.util.*;
import java.io.*;

import javax.swing.*;

import java.awt.*;
import java.awt.event.*;

public class GBADPreferences extends JDialog implements WindowListener
{
	// place to store the property file
	private String PROPERTY_DIR;
	private String PROPERTY_FILE;
	
	// directory were the packaged exectuables will be located
	public static final String BIN_DIR = getBinDir();
	
	// names for all used graphviz programs
	public static final String DOT = "dot";
	public static final String NEATO = "neato";
	public static final String TWOPI = "twopi";
	public static final String CIRCO = "circo";
	public static final String FDP = "fdp";
	public static final String SFDP = "sfdp";
	
	public static final String GRAPH2DOT = "graph2dot";
	public static final String GBAD = "gbad";
	
	private JPanel dotPathPanel;
	private JLabel dotPathLabel;
	private JTextField dotPathTextField;
	private String dotPathDescription = "Path to the dot executable.";
	
	private JPanel neatoPathPanel;
	private JLabel neatoPathLabel;
	private JTextField neatoPathTextField;
	private String neatoPathDescription = "Path to the neato executable.";
	
	private JPanel twopiPathPanel;
	private JLabel twopiPathLabel;
	private JTextField twopiPathTextField;
	private String twopiPathDescription = "Path to the twopi executable.";
	
	private JPanel circoPathPanel;
	private JLabel circoPathLabel;
	private JTextField circoPathTextField;
	private String circoPathDescription = "Path to the circo executable.";
	
	private JPanel fdpPathPanel;
	private JLabel fdpPathLabel;
	private JTextField fdpPathTextField;
	private String fdpPathDescription = "Path to the fdp executable.";
	
	private JPanel sfdpPathPanel;
	private JLabel sfdpPathLabel;
	private JTextField sfdpPathTextField;
	private String sfdpPathDescription = "Path to the sfdp executable.";
	
	private JPanel graph2dotPathPanel;
	private JLabel graph2dotPathLabel;
	private JTextField graph2dotPathTextField;
	private String graph2dotPathDescription = "Path to the graph2dot executable.";
	
	private JPanel gbadPathPanel;
	private JLabel gbadPathLabel;
	private JTextField gbadPathTextField;
	private String gbadPathDescription = "Path to the gbad executable.";
	
	private JPanel saveRestorePanel;
	private JButton save;
	private JButton cancel;
	private JButton restoreDefaults;
	
	public GBADPreferences()
	{
		super((JFrame)null, "Preferences");
		setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
		
		addWindowListener(this);
		
		setPropertyDir();
		
		setupLookAndFeel();
		
		addContent();
		
		loadPreferences();
	}
	
	public GBADPreferences(JFrame owner)
	{
		super(owner, "Preferences");
		setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
		
		addWindowListener(this);
		
		setPropertyDir();
		
		setupLookAndFeel();
		
		addContent();
		
		loadPreferences();
	}
	
	private void setPropertyDir()
	{
		try
		{
			PROPERTY_DIR = System.getProperty("user.home");
			if(PROPERTY_DIR == null)
			{
				throw new SecurityException();
			}
			
			PROPERTY_DIR += "/.GBAD/";
			(new File(PROPERTY_DIR)).mkdirs();
			
			PROPERTY_FILE = PROPERTY_DIR + "GUI.properties";
		}
		catch(SecurityException e)
		{
			PROPERTY_FILE = "GUI.properties";
			PROPERTY_DIR = ".";
		}
	}
	
	/*
	returns the bin directory
	this is the directory were the package exeuctables are located
	*/
	public static String getBinDir()
	{
		File file;
		
		try
		{
			String cwd = System.getProperty("user.dir");
			if(cwd == null)
			{
				throw new Exception();
			}
			
			file = new File(cwd + "/bin");
		}
		catch(Exception e)
		{
			file = new File("./bin");
		}
		
		System.out.println("bid_dir: " + file);
		
		//return file.toString();
		return "";
	}
	
	private void setupLookAndFeel()
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
		
		// if the OS is OSX then put the menu bar at the top of the screen
		// otherwise leave it in the application
		if(os.equals("Mac OS X"))
		{
			System.setProperty("apple.laf.useScreenMenuBar", "true");
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
	
	// add all of the conrent to this jframe
	private void addContent()
	{
		JPanel content = new JPanel();
		GridBagLayout gridbag = new GridBagLayout();
		content.setLayout(gridbag);
		
		dotPathLabel = new JLabel("DOT Executable:");
		dotPathLabel.setToolTipText(dotPathDescription);
		dotPathTextField = new JTextField(10);
		dotPathTextField.addMouseListener(new PathMouseListener(this, dotPathTextField));
		dotPathTextField.setEditable(false);
		//dotPathTextField.setText((new File(BIN_DIR, DOT)).toString());
		dotPathTextField.setText(DOT);
		dotPathTextField.setToolTipText(dotPathTextField.getText());
		addJTextFieldPanel(content, gridbag, dotPathLabel, dotPathTextField, DOT);
		
		neatoPathLabel = new JLabel("NEATO Executable:");
		neatoPathLabel.setToolTipText(neatoPathDescription);
		neatoPathTextField = new JTextField(10);
		neatoPathTextField.addMouseListener(new PathMouseListener(this, neatoPathTextField));
		neatoPathTextField.setEditable(false);
		//neatoPathTextField.setText((new File(BIN_DIR, NEATO)).toString());
		neatoPathTextField.setText(NEATO);
		neatoPathTextField.setToolTipText(neatoPathTextField.getText());
		addJTextFieldPanel(content, gridbag, neatoPathLabel, neatoPathTextField, NEATO);
		
		twopiPathLabel = new JLabel("TWOPI Executable:");
		twopiPathLabel.setToolTipText(twopiPathDescription);
		twopiPathTextField = new JTextField(10);
		twopiPathTextField.addMouseListener(new PathMouseListener(this, twopiPathTextField));
		twopiPathTextField.setEditable(false);
		//twopiPathTextField.setText((new File(BIN_DIR, TWOPI)).toString());
		twopiPathTextField.setText(TWOPI);
		twopiPathTextField.setToolTipText(twopiPathTextField.getText());
		addJTextFieldPanel(content, gridbag, twopiPathLabel, twopiPathTextField, TWOPI);
		
		circoPathLabel = new JLabel("CIRCO Executable:");
		circoPathLabel.setToolTipText(circoPathDescription);
		circoPathTextField = new JTextField(10);
		circoPathTextField.addMouseListener(new PathMouseListener(this, circoPathTextField));
		circoPathTextField.setEditable(false);
		//circoPathTextField.setText((new File(BIN_DIR, CIRCO)).toString());
		twopiPathTextField.setText(CIRCO);
		circoPathTextField.setToolTipText(circoPathTextField.getText());
		addJTextFieldPanel(content, gridbag, circoPathLabel, circoPathTextField, CIRCO);
		
		fdpPathLabel = new JLabel("FDP Executable:");
		fdpPathLabel.setToolTipText(fdpPathDescription);
		fdpPathTextField = new JTextField(10);
		fdpPathTextField.addMouseListener(new PathMouseListener(this, fdpPathTextField));
		fdpPathTextField.setEditable(false);
		//fdpPathTextField.setText((new File(BIN_DIR, FDP)).toString());
		twopiPathTextField.setText(FDP);
		fdpPathTextField.setToolTipText(fdpPathTextField.getText());
		addJTextFieldPanel(content, gridbag, fdpPathLabel, fdpPathTextField, FDP);
		
		sfdpPathLabel = new JLabel("SFDP Executable:");
		sfdpPathLabel.setToolTipText(sfdpPathDescription);
		sfdpPathTextField = new JTextField(10);
		sfdpPathTextField.addMouseListener(new PathMouseListener(this, sfdpPathTextField));
		sfdpPathTextField.setEditable(false);
		//sfdpPathTextField.setText((new File(BIN_DIR, SFDP)).toString());
		twopiPathTextField.setText(SFDP);
		sfdpPathTextField.setToolTipText(sfdpPathTextField.getText());
		addJTextFieldPanel(content, gridbag, sfdpPathLabel, sfdpPathTextField, SFDP);
		
		graph2dotPathLabel = new JLabel("Graph2Dot Executable:");
		graph2dotPathLabel.setToolTipText(graph2dotPathDescription);
		graph2dotPathTextField = new JTextField(10);
		graph2dotPathTextField.addMouseListener(new PathMouseListener(this, graph2dotPathTextField));
		graph2dotPathTextField.setEditable(false);
		//graph2dotPathTextField.setText((new File(BIN_DIR, GRAPH2DOT)).toString());
		twopiPathTextField.setText(GRAPH2DOT);
		graph2dotPathTextField.setToolTipText(graph2dotPathLabel.getText());
		addJTextFieldPanel(content, gridbag, graph2dotPathLabel, graph2dotPathTextField, GRAPH2DOT);
		
		gbadPathLabel = new JLabel("GBAD Executable:");
		gbadPathLabel.setToolTipText(gbadPathDescription);
		gbadPathTextField = new JTextField(10);
		gbadPathTextField.addMouseListener(new PathMouseListener(this, gbadPathTextField));
		gbadPathTextField.setEditable(false);
		//gbadPathTextField.setText((new File(BIN_DIR, GBAD)).toString());
		twopiPathTextField.setText(GBAD);
		gbadPathTextField.setToolTipText(gbadPathTextField.getText());
		addJTextFieldPanel(content, gridbag, gbadPathLabel, gbadPathTextField, GBAD);
		
		save = new JButton("Save");
		save.addActionListener(new SaveActionListener(this));
		
		cancel = new JButton("Cancel");
		cancel.addActionListener(new CancelActionListener(this));
		
		restoreDefaults = new JButton("Restore Defaults");
		restoreDefaults.addActionListener(new RestoreDefaultsActionListener(this));
		
		addRestorePanel(content, gridbag, save, cancel, restoreDefaults);
		
		JScrollPane scroller = new JScrollPane(content);
		scroller.setBorder(null);
		add(scroller);
		
		pack();
		setSize(getPreferredSize());
	}
	
	private void addJTextFieldPanel(JPanel content, GridBagLayout gridbag, JLabel label, JTextField textField, String name)
	{
		GridBagConstraints c = new GridBagConstraints();
		c.insets = new Insets(5,5,5,5);
		
		c.anchor = GridBagConstraints.WEST;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 0.0;
		gridbag.setConstraints(label, c);
		content.add(label);
		
		c.anchor = GridBagConstraints.CENTER;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.gridwidth = GridBagConstraints.RELATIVE;
		c.weightx = 1.0;
		gridbag.setConstraints(textField, c);
		content.add(textField);
		
		JButton restore = new JButton("Default " + name);
		//restore.addActionListener(new RestoreNamedDefaultActionListener(textField, (new File(BIN_DIR, name)).toString()));
		restore.addActionListener(new RestoreNamedDefaultActionListener(textField, name));
		
		c.anchor = GridBagConstraints.EAST;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.weightx = 0.0;
		gridbag.setConstraints(restore, c);
		content.add(restore);
	}
	
	private void addRestorePanel(JPanel content, GridBagLayout gridbag, JButton save, JButton cancel, JButton restoreDefaults)
	{
		GridBagConstraints c = new GridBagConstraints();
		c.insets = new Insets(5,5,5,5);
		
		c.fill = GridBagConstraints.NONE;
		c.weightx = 0.0;
		c.weighty = 1.0;
		
		c.anchor = GridBagConstraints.SOUTHWEST;
		gridbag.setConstraints(save, c);
		content.add(save);
		
		c.anchor = GridBagConstraints.SOUTH;
		c.gridheight = GridBagConstraints.REMAINDER;
		gridbag.setConstraints(cancel, c);
		content.add(cancel);
		
		c.anchor = GridBagConstraints.SOUTHEAST;
		c.gridwidth = GridBagConstraints.REMAINDER;
		gridbag.setConstraints(restoreDefaults, c);
		content.add(restoreDefaults);
	}
	
	/*
	Load the preferences file into the system properties and set the text fields.
	*/
	protected void loadPreferences()
	{
		try
		{
			String user_dir = System.getProperty("user.dir");
			String user_home = System.getProperty("user.home");
			String os_name = System.getProperty("os.name");
		
			System.out.println("cwd is " + user_dir);
			System.out.println("load " + PROPERTY_FILE);
			System.getProperties().load(new FileInputStream(PROPERTY_FILE));
			
			System.setProperty("user.dir", user_dir);
			System.setProperty("user.home", user_home);
			System.setProperty("os.name", os_name);
			
			dotPathTextField.setText(System.getProperty(DOT));
			dotPathTextField.setToolTipText(dotPathTextField.getText());
			
			neatoPathTextField.setText(System.getProperty(NEATO));
			neatoPathTextField.setToolTipText(neatoPathTextField.getText());
			
			twopiPathTextField.setText(System.getProperty(TWOPI));
			twopiPathTextField.setToolTipText(twopiPathTextField.getText());
			
			circoPathTextField.setText(System.getProperty(CIRCO));
			circoPathTextField.setToolTipText(circoPathTextField.getText());
			
			fdpPathTextField.setText(System.getProperty(FDP));
			fdpPathTextField.setToolTipText(fdpPathTextField.getText());
			
			sfdpPathTextField.setText(System.getProperty(SFDP));
			sfdpPathTextField.setToolTipText(sfdpPathTextField.getText());
			
			graph2dotPathTextField.setText(System.getProperty(GRAPH2DOT));
			graph2dotPathTextField.setToolTipText(graph2dotPathTextField.getText());
			
			gbadPathTextField.setText(System.getProperty(GBAD));
			gbadPathTextField.setToolTipText(gbadPathTextField.getText());
		}
		catch(SecurityException se)
		{
			JOptionPane.showMessageDialog(this, "Unable To Load Preferences", "Security Exception", JOptionPane.ERROR_MESSAGE);
		}
		catch(IOException ioe)
		{
			// failed to load property file
			// setup defaults and try to create one
			
			try
			{
				restoreDefaults();
				
				System.setProperty("property_file", PROPERTY_FILE);
				
				savePreferences();
			}
			catch(SecurityException se)
			{
				JOptionPane.showMessageDialog(this, "Unable To Create Preference File", "Security Exception", JOptionPane.ERROR_MESSAGE);
			}
		}
	}
	
	/*
	Reset the textfields to hold the users preferences (not the defaults).
	*/
	protected void resetTextFields()
	{
		try
		{
			dotPathTextField.setText(System.getProperty(DOT));
			dotPathTextField.setToolTipText(dotPathTextField.getText());
			
			neatoPathTextField.setText(System.getProperty(NEATO));
			neatoPathTextField.setToolTipText(neatoPathTextField.getText());
			
			twopiPathTextField.setText(System.getProperty(TWOPI));
			twopiPathTextField.setToolTipText(twopiPathTextField.getText());
			
			circoPathTextField.setText(System.getProperty(CIRCO));
			circoPathTextField.setToolTipText(circoPathTextField.getText());
			
			fdpPathTextField.setText(System.getProperty(FDP));
			fdpPathTextField.setToolTipText(fdpPathTextField.getText());
			
			sfdpPathTextField.setText(System.getProperty(SFDP));
			sfdpPathTextField.setToolTipText(sfdpPathTextField.getText());
			
			graph2dotPathTextField.setText(System.getProperty(GRAPH2DOT));
			graph2dotPathTextField.setToolTipText(graph2dotPathTextField.getText());
			
			gbadPathTextField.setText(System.getProperty(GBAD));
			gbadPathTextField.setToolTipText(gbadPathTextField.getText());
		}
		catch(SecurityException se)
		{
			JOptionPane.showMessageDialog(this, "Unable to load default preferences.", "Security Exception", JOptionPane.ERROR_MESSAGE);
		}
	}
	
	/*
	Sets the textfiled to the default exe locations (in BIN_DIR).
	*/
	protected void restoreDefaults() throws SecurityException
	{
		//dotPathTextField.setText((new File(BIN_DIR, DOT).toString()));
		dotPathTextField.setText(DOT);
		dotPathTextField.setToolTipText(dotPathTextField.getText());
		
		//neatoPathTextField.setText((new File(BIN_DIR, NEATO).toString()));
		neatoPathTextField.setText(NEATO);
		neatoPathTextField.setToolTipText(neatoPathTextField.getText());
		
		//twopiPathTextField.setText((new File(BIN_DIR, TWOPI).toString()));
		twopiPathTextField.setText(TWOPI);
		twopiPathTextField.setToolTipText(twopiPathTextField.getText());
		
		//circoPathTextField.setText((new File(BIN_DIR, CIRCO).toString()));
		circoPathTextField.setText(CIRCO);
		circoPathTextField.setToolTipText(circoPathTextField.getText());
		
		//fdpPathTextField.setText((new File(BIN_DIR, FDP).toString()));
		fdpPathTextField.setText(FDP);
		fdpPathTextField.setToolTipText(fdpPathTextField.getText());
		
		//sfdpPathTextField.setText((new File(BIN_DIR, SFDP).toString()));
		sfdpPathTextField.setText(SFDP);
		sfdpPathTextField.setToolTipText(sfdpPathTextField.getText());
		
		//graph2dotPathTextField.setText((new File(BIN_DIR, GRAPH2DOT).toString()));
		graph2dotPathTextField.setText(GRAPH2DOT);
		graph2dotPathTextField.setToolTipText(graph2dotPathTextField.getText());
		
		//gbadPathTextField.setText((new File(BIN_DIR, GBAD).toString()));
		gbadPathTextField.setText(GBAD);
		gbadPathTextField.setToolTipText(gbadPathTextField.getText());
	}
		
	protected void savePreferences()
	{
		try
		{
			System.setProperty(DOT, dotPathTextField.getText());
			System.setProperty(NEATO, neatoPathTextField.getText());
			System.setProperty(TWOPI, twopiPathTextField.getText());
			System.setProperty(CIRCO, circoPathTextField.getText());
			System.setProperty(FDP, fdpPathTextField.getText());
			System.setProperty(SFDP, sfdpPathTextField.getText());
			System.setProperty(GRAPH2DOT, graph2dotPathTextField.getText());
			System.setProperty(GBAD, gbadPathTextField.getText());

			System.getProperties().store(new FileOutputStream(PROPERTY_FILE), null);
			setVisible(false);
		}
		catch(SecurityException se)
		{
			JOptionPane.showMessageDialog(this, "Unable to set preferences.", "Security Exception", JOptionPane.ERROR_MESSAGE);
		}
		catch(IOException ioe)
		{
			JOptionPane.showMessageDialog(this, "Unable to save preference file.", "Error Creating Preference File", JOptionPane.ERROR_MESSAGE);
		}
	}
	
	public void windowClosing(WindowEvent e)
	{
		resetTextFields();
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

/*
ActionListener for the save button.
*/
class SaveActionListener implements ActionListener
{
	private GBADPreferences preferences;
	
	public SaveActionListener(GBADPreferences preferences)
	{
		this.preferences = preferences;
	}
	
	public void actionPerformed(ActionEvent e)
	{
		preferences.savePreferences();
	}
}

/*
ActionListener for the cancel button.
*/
class CancelActionListener implements ActionListener
{
	private GBADPreferences preferences;
	
	public CancelActionListener(GBADPreferences preferences)
	{
		this.preferences = preferences;
	}
	
	public void actionPerformed(ActionEvent e)
	{
		preferences.resetTextFields();
		preferences.setVisible(false);
	}
}

/*
ActionListener for the restore defaults button.
*/
class RestoreDefaultsActionListener implements ActionListener
{
	private GBADPreferences preferences;
	
	public RestoreDefaultsActionListener(GBADPreferences preferences)
	{
		this.preferences = preferences;
	}
	
	public void actionPerformed(ActionEvent e)
	{
		preferences.restoreDefaults();
	}
}

/*
ActionListener for a specific restore default button.
*/
class RestoreNamedDefaultActionListener implements ActionListener
{
	private String name;
	private JTextField textField;
	
	public RestoreNamedDefaultActionListener(JTextField textField, String name)
	{
		this.textField = textField;
		this.name = name;
	}
	
	public void actionPerformed(ActionEvent e)
	{
		textField.setText(name);
		textField.setToolTipText(name);
	}
}

/*
MouseListener that pulls up a JFileChooser when the path is clicked.
*/
class PathMouseListener implements MouseListener
{
	private static File lastDir;
	
	private Component parent;
	private JTextField textField;
	
	public PathMouseListener(Component parent, JTextField textField)
	{
		this.parent = parent;
		this.textField = textField;
		
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
	
	public void mousePressed(MouseEvent e)
	{
		
	}
	
	public void mouseReleased(MouseEvent e)
	{
		
	}
	
	public void mouseEntered(MouseEvent e)
	{
		
	}
	
	public void mouseExited(MouseEvent e)
	{
		
	}
	
	public void mouseClicked(MouseEvent e)
	{
		try
		{
			JFileChooser chooser = new JFileChooser(lastDir);
			
			int returnVal = chooser.showOpenDialog(parent);
			if(returnVal == JFileChooser.APPROVE_OPTION)
			{
				File tempFile = chooser.getSelectedFile();
				
				if(tempFile.getParentFile() != null)
				{
					lastDir = tempFile.getParentFile();
				}
				
				System.out.println(" SELECTED: " + tempFile.toString());
				textField.setText(tempFile.toString());
				textField.setToolTipText(tempFile.toString());
			}
		}
		catch(Exception ex)
		{
			JOptionPane.showMessageDialog(parent, "Unable to access user files.", "Access Control Exception", JOptionPane.ERROR_MESSAGE);
		}
	}
} 
