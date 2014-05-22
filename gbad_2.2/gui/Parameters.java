import java.util.Vector;

import java.awt.Component;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;

import javax.swing.Box;
import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.DefaultListCellRenderer;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextField;
import javax.swing.SwingConstants;

import javax.swing.border.TitledBorder;

import javax.swing.event.EventListenerList;


/*
This class handels the layout of the Parameters section of the GUI.
*/
public class Parameters extends JPanel implements ActionListener
{
	private JComboBox anomalousDetectionComboBox;
	private String[] anomalousDetectionValues = {"None", "-mdl", "-mps", "-prob"};
	private String anomalousDetectionDescription = "Anomaly detection algorithm used.";
	
	private JLabel mdlLabel;
	private JTextField mdlTextField;
	private String mdlDefault = "0.3";
	private String mdlDescription = "Percentage of change allowed (0.0 - 1.0).";
	
	private JLabel mpsLabel;
	private JTextField mpsTextField;
	private String mpsDefault = "0.3";
	private String mpsDescription = "Percentage of change allowed (0.0 - 1.0).";
	
	private JLabel probLabel;
	private JTextField probTextField;
	private String probDefault = "2";
	private String probDescription = "Number of iterations.";
	
	private JLabel beamLabel;
	private JTextField beamTextField;
	private String beamDefault = "4";
	private String beamDescription = "See GBAD manual.";
	
	private JLabel compressLabel;
	private JCheckBox compressCheckBox;
	private String compressDescription = "Writes the compressed data to a file.  For typical SUBDUE processing, the file <graph>.g.cmp is created (ex. sample.g.cmp)";
	
	private JComboBox evalComboBox;
	private String[] evalValues = {"MDL (-eval 1)", "Size (-eval 2)", "Set Cover (-eval 3)"};
	private String evalDescription = "Method used for evaluating candidate substructures.  See SUBDUE manual.";
	
	private JLabel incLabel;
	private JCheckBox incCheckBox;
	private String incDescription = "Incremental data handleing.  See GBAD manual.";
	
	private JLabel iterationsLabel;
	private JTextField iterationsTextField;
	private String iterationsDefault = "1";
	private String iterationsDescription = "The number iterations over the input graph.";
	
	private JLabel limitLabel;
	private JTextField limitTextField;
	private String limitDefault = "( |E(G)| / 2 )";
	private String limitDescription = "The number of different substructures to consider in each iteration.";
	
	private JLabel maxAnomScoreLabel;
	private JTextField maxAnomScoreTextField;
	private String maxAnomScoreDefault = "MAX";
	private String maxAnomScoreDescription = "Maximum Anomalous socre for reporting potential anomalies.";
	
	private JLabel minAnomScoreLabel;
	private JTextField minAnomScoreTextField;
	private String minAnomScoreDefault = "MIN";
	private String minAnomScoreDescription = "Minimum Anomalous socre for reporting potential anomalies).";
	
	private JLabel maxSizeLabel;
	private JTextField maxSizeTextField;
	private String maxSizeDefault = "( |V(G)| )";
	private String maxSizeDescription = "Maximum number of vertices allowed in each considered substructure.";
	
	private JLabel minSizeLabel;
	private JTextField minSizeTextField;
	private String minSizeDefault = "1";
	private String minSizeDescription = "Minimum number of vertices allowed in each considered substructures.";

	private JLabel nSubsLabel;
	private JTextField nSubsTextField;
	private String nSubsDefault = "3";
	private String nSubsDescription = "Number of best substructures stored.";
	
	private JLabel outputLabel;
	private JTextField outputTextField;
	private String outputDefault = "2";
	private String outputDescription = "Output Level.  See GBAD manual.";
	
	private JLabel overlapLabel;
	private JCheckBox overlapCheckBox;
	private String overlapDescription = "Allow overlaping of substructures.";
	
	private JLabel pruneLabel;
	private JCheckBox pruneCheckBox;
	private String pruneDescription = "Turn purning on.  See SUBDUE manual.";
	
	private JLabel undirectedLabel;
	private JCheckBox undirectedCheckBox;
	private String undirectedDescription = "Treat edges labled as \"e\" as undirected.";
	
	private JLabel valuebasedLabel;
	private JCheckBox valuebasedCheckBox;
	private String valuebasedDescription = "See GBAD manual.";
	
	private UpdateGBADCount gbadCountLabel;
	
	private JButton restoreDefaults;
	private String restoreDefaultsText = "Restore Defaults";
	
	private JButton advanced;
	public static final String advancedText = "Advanced >>";
	public static final String simpleText = "<< Simple";
	
	private JButton startGBAD;
	public static final String startGBADText = "Start GBAD";
	
	EventListenerList listenerList;
	
	public Parameters()
	{
		super();
		
		listenerList = new EventListenerList();
		
		setBorder(BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.blue), "Parameters", TitledBorder.LEFT, TitledBorder.TOP, new Font("Comic Sans MS", Font.BOLD, 15), Color.red));
		
		addContent();

		// hide the advanced parameters
		hideParameters(true);
	}
	
	// create and add all content to this jpanel
	private void addContent()
	{
		JPanel content = new JPanel();
		GridBagLayout gridbag = new GridBagLayout();
		content.setLayout(gridbag);
		
		addAHack(content, gridbag);
		
		anomalousDetectionComboBox = new JComboBox(anomalousDetectionValues);
		anomalousDetectionComboBox.setRenderer(new ListCellRenderer());
		anomalousDetectionComboBox.setToolTipText(anomalousDetectionDescription);
		addJComboBoxPanel(content, gridbag, anomalousDetectionComboBox);
		
		mdlLabel = new JLabel("deviation");
		mdlTextField = new JTextField(10);
		mdlTextField.addKeyListener(new PercentKeyListener(mdlDescription));
		mdlLabel.setToolTipText(mdlDescription);
		mdlTextField.setToolTipText(mdlDescription);
		addJTextFieldPanel(content, gridbag, mdlLabel, mdlTextField);
		mdlLabel.setVisible(false);
		mdlTextField.setVisible(false);
		
		mpsLabel = new JLabel("deviation");
		mpsTextField = new JTextField(10);
		mpsTextField.addKeyListener(new PercentKeyListener(mpsDescription));
		mpsLabel.setToolTipText(mpsDescription);
		mpsTextField.setToolTipText(mpsDescription);
		addJTextFieldPanel(content, gridbag, mpsLabel, mpsTextField);
		mpsLabel.setVisible(false);
		mpsTextField.setVisible(false);
		
		probLabel = new JLabel("iterations");
		probTextField = new JTextField(10);
		probTextField.addKeyListener(new GreaterThanOneKeyListener(probDescription));
		probLabel.setToolTipText(probDescription);
		probTextField.setToolTipText(probDescription);
		addJTextFieldPanel(content, gridbag, probLabel, probTextField);
		probLabel.setVisible(false);
		probTextField.setVisible(false);
		
		anomalousDetectionComboBox.addItemListener(new StateChangeListener(mdlLabel, mdlTextField, mpsLabel, mpsTextField, probLabel, probTextField));
		
		evalComboBox = new JComboBox(evalValues);
		evalComboBox.setRenderer(new ListCellRenderer());
		evalComboBox.setToolTipText(evalDescription);
		addJComboBoxPanel(content, gridbag, evalComboBox);
		
		beamLabel = new JLabel("-beam");
		beamTextField = new JTextField(10);
		beamTextField.addKeyListener(new PostiveIntegerKeyListener(beamDescription));
		beamLabel.setToolTipText(beamDescription);
		beamTextField.setToolTipText(beamDescription);
		addJTextFieldPanel(content, gridbag, beamLabel, beamTextField);
		
		compressLabel = new JLabel("-compress");
		compressCheckBox = new JCheckBox("");
		compressLabel.setToolTipText(compressDescription);
		compressCheckBox.setToolTipText(compressDescription);
		addJCheckBoxPanel(content, gridbag, compressLabel, compressCheckBox);
		
		incLabel = new JLabel("-inc");
		incCheckBox = new JCheckBox("");
		incLabel.setToolTipText(incDescription);
		incCheckBox.setToolTipText(incDescription);
		addJCheckBoxPanel(content, gridbag, incLabel, incCheckBox);
		
		iterationsLabel = new JLabel("-iterations");
		iterationsTextField = new JTextField(10);
		iterationsTextField.addKeyListener(new PostiveIntegerKeyListener(iterationsDescription));
		iterationsLabel.setToolTipText(iterationsDescription);
		iterationsTextField.setToolTipText(iterationsDescription);
		addJTextFieldPanel(content, gridbag, iterationsLabel, iterationsTextField);
		
		limitLabel = new JLabel("-limit");
		limitTextField = new JTextField(10);
		limitTextField.addKeyListener(new LimitKeyListener(limitDescription));
		limitLabel.setToolTipText(limitDescription);
		limitTextField.setToolTipText(limitDescription);
		addJTextFieldPanel(content, gridbag, limitLabel, limitTextField);
		
		maxAnomScoreLabel = new JLabel("-maxAnomalousScore");
		maxAnomScoreTextField = new JTextField(10);
		maxAnomScoreTextField.addKeyListener(new MaxKeyListener(maxAnomScoreDescription));
		maxAnomScoreLabel.setToolTipText(maxAnomScoreDescription);
		maxAnomScoreTextField.setToolTipText(maxAnomScoreDescription);
		addJTextFieldPanel(content, gridbag, maxAnomScoreLabel, maxAnomScoreTextField);
		
		minAnomScoreLabel = new JLabel("-minAnomalousScore");
		minAnomScoreTextField = new JTextField(10);
		minAnomScoreTextField.addKeyListener(new MinKeyListener(minAnomScoreDescription));
		minAnomScoreLabel.setToolTipText(minAnomScoreDescription);
		minAnomScoreTextField.setToolTipText(minAnomScoreDescription);
		addJTextFieldPanel(content, gridbag, minAnomScoreLabel, minAnomScoreTextField);
		
		maxSizeLabel = new JLabel("-maxsize");
		maxSizeTextField = new JTextField(10);
		maxSizeTextField.addKeyListener(new PostiveIntegerKeyListener(maxSizeDescription));
		maxSizeLabel.setToolTipText(maxSizeDescription);
		maxSizeTextField.setToolTipText(maxSizeDescription);
		addJTextFieldPanel(content, gridbag, maxSizeLabel, maxSizeTextField);
		
		minSizeLabel = new JLabel("-minsize");
		minSizeTextField = new JTextField(10);
		minSizeTextField.addKeyListener(new PostiveIntegerKeyListener(minSizeDescription));
		minSizeLabel.setToolTipText(minSizeDescription);
		minSizeTextField.setToolTipText(minSizeDescription);
		addJTextFieldPanel(content, gridbag, minSizeLabel, minSizeTextField);
		
		nSubsLabel = new JLabel("-nsubs");
		nSubsTextField = new JTextField(10);
		nSubsTextField.addKeyListener(new PostiveIntegerKeyListener(nSubsDescription));
		nSubsLabel.setToolTipText(nSubsDescription);
		nSubsTextField.setToolTipText(nSubsDescription);
		addJTextFieldPanel(content, gridbag, nSubsLabel, nSubsTextField);
		
		outputLabel = new JLabel("-output");
		outputTextField = new JTextField(10);
		outputTextField.addKeyListener(new OneToFiveKeyListener(outputDescription));
		outputLabel.setToolTipText(outputDescription);
		outputTextField.setToolTipText(outputDescription);
		addJTextFieldPanel(content, gridbag, outputLabel, outputTextField);
		
		overlapLabel = new JLabel("-overlap");
		overlapCheckBox = new JCheckBox("");
		overlapLabel.setToolTipText(overlapDescription);
		overlapCheckBox.setToolTipText(overlapDescription);
		addJCheckBoxPanel(content, gridbag, overlapLabel, overlapCheckBox);
		
		pruneLabel = new JLabel("-prune");
		pruneCheckBox = new JCheckBox("");
		pruneLabel.setToolTipText(pruneDescription);
		pruneCheckBox.setToolTipText(pruneDescription);
		addJCheckBoxPanel(content, gridbag, pruneLabel, pruneCheckBox);
		
		undirectedLabel = new JLabel("-undirected");
		undirectedCheckBox = new JCheckBox("");
		undirectedLabel.setToolTipText(undirectedDescription);
		undirectedCheckBox.setToolTipText(undirectedDescription);
		addJCheckBoxPanel(content, gridbag, undirectedLabel, undirectedCheckBox);
		
		valuebasedLabel = new JLabel("-valuebased");
		valuebasedCheckBox = new JCheckBox("");
		valuebasedLabel.setToolTipText(valuebasedDescription);
		valuebasedCheckBox.setToolTipText(valuebasedDescription);
		addJCheckBoxPanel(content, gridbag, valuebasedLabel, valuebasedCheckBox);
		
		/*
		// the following code reads in an image and proints out its raw
		// data from as a java array
		
		
		byte[] data = new byte[0];
		try
		{
			FileInputStream reader = new FileInputStream("gbad.png");
			data = new byte[reader.available()];
			reader.read(data);
		}
		catch(Exception e)
		{
		
		}
		
		System.out.print("{");
		for(int i=0; i < data.length; i++)
		{
			System.out.print(data[i] + ",");
		}
		System.out.println("}");
		*/

		gbadCountLabel = new UpdateGBADCount();
		(new Thread(gbadCountLabel)).start();
		
		restoreDefaults();
		
		restoreDefaults = new JButton(restoreDefaultsText);
		restoreDefaults.setActionCommand(restoreDefaultsText);
		restoreDefaults.addActionListener(this);
		
		advanced = new JButton(advancedText);
		advanced.setActionCommand(advancedText);
		advanced.addActionListener(this);
		
		startGBAD = new JButton(startGBADText);
		startGBAD.setActionCommand(startGBADText);
		startGBAD.addActionListener(this);
		
		JButton[] buttons = {restoreDefaults, advanced, startGBAD};
		addJButtonPanel(content, gridbag, gbadCountLabel, buttons);
		
		JScrollPane scroller = new JScrollPane(content);
		
		setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
		add(scroller);
		
	}
	
	// this function restores the default values of every parameter
	private void restoreDefaults()
	{
		anomalousDetectionComboBox.setSelectedIndex(0);
		mdlTextField.setText(mdlDefault);
		mpsTextField.setText(mpsDefault);
		probTextField.setText(probDefault);
		beamTextField.setText(beamDefault);
		compressCheckBox.setSelected(false);
		evalComboBox.setSelectedIndex(0);
		incCheckBox.setSelected(false);
		iterationsTextField.setText(iterationsDefault);
		limitTextField.setText(limitDefault);
		maxAnomScoreTextField.setText(maxAnomScoreDefault);
		minAnomScoreTextField.setText(minAnomScoreDefault);
		maxSizeTextField.setText(maxSizeDefault);
		minSizeTextField.setText(minSizeDefault);
		nSubsTextField.setText(nSubsDefault);
		outputTextField.setText(outputDefault);
		overlapCheckBox.setSelected(false);
		pruneCheckBox.setSelected(false);
		undirectedCheckBox.setSelected(false);
		valuebasedCheckBox.setSelected(false);
		
		beamTextField.setBackground(new Color(255, 255, 255));
		iterationsTextField.setBackground(new Color(255, 255, 255));
		limitTextField.setBackground(new Color(255, 255, 255));
		maxAnomScoreTextField.setBackground(new Color(255, 255, 255));
		minAnomScoreTextField.setBackground(new Color(255, 255, 255));
		maxSizeTextField.setBackground(new Color(255, 255, 255));
		minSizeTextField.setBackground(new Color(255, 255, 255));
		nSubsTextField.setBackground(new Color(255, 255, 255));
		outputTextField.setBackground(new Color(255, 255, 255));
	}
	
	/*
	Add an invisible component to the layout that should be larger than
	any other component.  This forces a consistent minimum width no matter
	what is currently being displayed.
	*/
	private void addAHack(JPanel content, GridBagLayout gridbag)
	{
		GridBagConstraints c = new GridBagConstraints();
		c.insets = new Insets(0,1,5,1);
		
		Component aHack = Box.createRigidArea(new Dimension(275, 0));
		
		c.anchor = GridBagConstraints.CENTER;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.weightx = 1.0;
		gridbag.setConstraints(aHack, c);
		content.add(aHack);
	}
	
	// Add an input JTextField and a parameter description JLabel to the content.
	private void addJTextFieldPanel(JPanel content, GridBagLayout gridbag, JLabel label, JTextField textField)
	{
		GridBagConstraints c = new GridBagConstraints();
		c.insets = new Insets(0,1,5,1);
		
		c.anchor = GridBagConstraints.WEST;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 0.0;
		gridbag.setConstraints(label, c);
		content.add(label);
		
		c.anchor = GridBagConstraints.WEST;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.weightx = 1.0;
		gridbag.setConstraints(textField, c);
		content.add(textField);

	}
	
	// Add an input JCheckBox and a parameter description JLabel to the content.
	private void addJCheckBoxPanel(JPanel content, GridBagLayout gridbag, JLabel label, JCheckBox checkBox)
	{
		GridBagConstraints c = new GridBagConstraints();
		c.insets = new Insets(0,1,5,1);
		
		c.anchor = GridBagConstraints.WEST;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 0.0;
		gridbag.setConstraints(label, c);
		content.add(label);
		
		c.anchor = GridBagConstraints.WEST;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.weightx = 1.0;
		gridbag.setConstraints(checkBox, c);
		content.add(checkBox);
	}
	
	// Add an input JComboxBox to the content.
	private void addJComboBoxPanel(JPanel content, GridBagLayout gridbag, JComboBox comboBox)
	{
		GridBagConstraints c = new GridBagConstraints();
		c.insets = new Insets(0,1,5,1);
		
		c.anchor = GridBagConstraints.CENTER;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.weightx = 1.0;
		gridbag.setConstraints(comboBox, c);
		content.add(comboBox);
	}
	
	/*
	This function sets up the bottom of the parameters jpanel.  It adds, in order,
	the JLabel, and then each of the buttons to the content.  The label and the buttons
	will always be found on the last few rows of the girdbag layout.
	*/
	private void addJButtonPanel(JPanel content, GridBagLayout gridbag, JLabel label, JButton[] buttons)
	{
		JPanel panel = new JPanel();
		GridBagLayout tempBag = new GridBagLayout();
		panel.setLayout(tempBag);
		
		GridBagConstraints c = new GridBagConstraints();
		c.insets = new Insets(2,2,2,2);
		
		c.anchor = GridBagConstraints.WEST;
		c.fill = GridBagConstraints.NONE;
		c.gridwidth = GridBagConstraints.REMAINDER;
		
		tempBag.setConstraints(label, c);
		panel.add(label);
		
		c.anchor = GridBagConstraints.WEST;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.weightx = 1.0;
		
		for(int index=0; index<buttons.length; index++)
		{
			tempBag.setConstraints(buttons[index], c);
			panel.add(buttons[index]);
		}
		
		c.anchor = GridBagConstraints.SOUTHWEST;
		c.fill = GridBagConstraints.NONE;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.weightx = 0.0;
		c.gridheight = GridBagConstraints.REMAINDER;
		c.weighty = 1.0;
		
		gridbag.setConstraints(panel, c);
		content.add(panel);
	}
	
	// Set any advanced parameters to the specified visibility.
	public void hideParameters(boolean visible)
	{
		beamLabel.setVisible(!visible);
		beamTextField.setVisible(!visible);
	
		compressLabel.setVisible(false);
		compressCheckBox.setVisible(false);
	
		evalComboBox.setVisible(!visible);
	
		incLabel.setVisible(false);
		incCheckBox.setVisible(false);
	
		iterationsLabel.setVisible(false);
		iterationsTextField.setVisible(false);
	
		limitLabel.setVisible(!visible);
		limitTextField.setVisible(!visible);
		
		maxAnomScoreLabel.setVisible(!visible);
		maxAnomScoreTextField.setVisible(!visible);
	
		minAnomScoreLabel.setVisible(!visible);
		minAnomScoreTextField.setVisible(!visible);
	
		maxSizeLabel.setVisible(!visible);
		maxSizeTextField.setVisible(!visible);
		
		minSizeLabel.setVisible(!visible);
		minSizeTextField.setVisible(!visible);
		
		nSubsLabel.setVisible(!visible);
		nSubsTextField.setVisible(!visible);
		
		outputLabel.setVisible(!visible);
		outputTextField.setVisible(!visible);
		
		overlapLabel.setVisible(!visible);
		overlapCheckBox.setVisible(!visible);
		
		pruneLabel.setVisible(!visible);
		pruneCheckBox.setVisible(!visible);
		
		undirectedLabel.setVisible(!visible);
		undirectedCheckBox.setVisible(!visible);
		
		valuebasedLabel.setVisible(!visible);
		valuebasedCheckBox.setVisible(!visible);
	}
	
	/*
	Try to read an integer out of the JTextField and paramString follows by the 
	integer to args.  If the text is not a valid integer a NumberFormatException
	is thrown.  
	*/
	private void addTextFieldIntParam(Vector<String> args, JTextField field, String paramString, String defaultString) throws NumberFormatException
	{
		if(paramString.equals("-lim"))
		{
			if(field.getText().equals(limitDefault))
			{
				return;
			}
		}
			
		try
		{
			int value = Integer.parseInt(field.getText());
			
			if(value < 0)
			{
				throw new NumberFormatException("");
			}
			
			args.add(paramString);
			args.add(field.getText());
		}
		catch(NumberFormatException nfe)
		{
			if((field != limitTextField) && (field != maxSizeTextField))
			{
				String value = field.getText();
				
				if(value.trim().equals(""))
				{
					value = "";
				}
				
				throw new NumberFormatException(paramString + " parameter (" + value + ") is not a valid positive integer.\nThe default for this parameter is " + defaultString + ".");
			}
		}
	}
	
	/*
	Try to read a number between 0.0 and 1.0 out of the JTextField and paramString
	follows by the integer to args.  If the text is not a valid integer a
	NumberFormatException is thrown.  
	*/
	private void addTextFieldDoubleParam(Vector<String> args, JTextField field, String paramString, String defaultString) throws NumberFormatException
	{
		try
		{
			double value = Double.parseDouble(field.getText());
			
			if((value < 0.0) || (value > 1.0))
			{
				throw new NumberFormatException("");
			}
			
			args.add(paramString);
			args.add(field.getText());
		}
		catch(NumberFormatException nfe)
		{
			if((field != limitTextField) && (field != maxSizeTextField))
			{
				String value = field.getText();
				
				if(value.trim().equals(""))
				{
					value = "";
				}
				
				throw new NumberFormatException(paramString + " parameter (" + value + ") is not valid.\nThe default for this parameter is " + defaultString + ".");
			}

		}
	}
	
	/*
	Try to read a number greater than 0.0 out of the JTextField and paramString
	follows by the integer to args.  If the text is not a valid integer a
	NumberFormatException is thrown.  
	*/
	private void addTextField_MIN_MAX_Param(Vector<String> args, JTextField field, String paramString, String defaultString) throws NumberFormatException
	{
		if(paramString.equals("-maxAnomalousScore"))
		{
			if(field.getText().equals(maxAnomScoreDefault))
			{
				return;
			}
		}
		else if(paramString.equals("-minAnomalousScore"))
		{
			if(field.getText().equals(minAnomScoreDefault))
			{
				return;
			}
		}
		
		System.out.println("HERE");
		
		try
		{
			double value = Double.parseDouble(field.getText());
			
			if(value < 0.0)
			{
				throw new NumberFormatException("");
			}
			
			args.add(paramString);
			args.add(field.getText());
		}
		catch(NumberFormatException nfe)
		{
			if((field != limitTextField) && (field != maxSizeTextField))
			{
				String value = field.getText();
				
				if(value.trim().equals(""))
				{
					value = "";
				}
				
				throw new NumberFormatException(paramString + " parameter (" + value + ") is not valid.\nThe default for this parameter is " + defaultString + ".");
			}

		}
	}
	
	/*
	If checkBox is checked, add paramString to args.
	*/
	private void appendCheckBoxParam(Vector<String> args, JCheckBox checkBox, String paramString)
	{
		if(checkBox.isSelected())
		{
			args.add(paramString);
		}
	}
	
	public String getAnomalyAlgorithm()
	{
		return anomalousDetectionComboBox.getSelectedItem().toString();
	}
	
	/*
	Return the selected item out of the anomalousDetectionComboBox.
	*/
	public void addAnomalyAlgorithm(Vector<String> args) throws NumberFormatException
	{
		String selected = getAnomalyAlgorithm();
		
		if(!selected.equals(anomalousDetectionValues[0]))
		{
			if(selected.equals("-mdl"))
			{
				addTextFieldDoubleParam(args, mdlTextField, "-mdl", mdlDefault);
			}
			else if(selected.equals("-mps"))
			{
				addTextFieldDoubleParam(args, mpsTextField, "-mps", mpsDefault);
			}
			else if(selected.equals("-prob"))
			{
				addTextFieldIntParam(args, probTextField, "-prob", probDefault);
			}
		}
	}
	
	/*
	returns a vector of all the user defined parameters needed to started bad
	*/
	public Vector<String> getParameters() throws NumberFormatException
	{
		Vector<String> args = new Vector<String>();
		
		String gbadEXE;
		
		try
		{
			gbadEXE = System.getProperty("gbad");
			if(gbadEXE == null)
			{
				throw new SecurityException();
			}
		}
		catch(SecurityException se)
		{
			gbadEXE = "gbad";
			JOptionPane.showMessageDialog(this, "Unable to determine location of gbad.  Assuming in path.", "Security Exception", JOptionPane.ERROR_MESSAGE);
		}
				
		args.add(gbadEXE);
		
		addAnomalyAlgorithm(args);
		
		addTextFieldIntParam(args, beamTextField, "-beam", beamDefault);
		//appendCheckBoxParam(args, compressCheckBox, "-compress");
		
		args.add("-eval");
		args.add("" + (evalComboBox.getSelectedIndex()+1));
		
		//appendCheckBoxParam(args, incCheckBox, "-inc");
		//addTextFieldIntParam(args, iterationsTextField, "-iterations", iterationsDefault);
		addTextFieldIntParam(args, limitTextField, "-limit", limitDefault);
		addTextField_MIN_MAX_Param(args, maxAnomScoreTextField, "-maxAnomalousScore", maxAnomScoreDefault);
		addTextField_MIN_MAX_Param(args, minAnomScoreTextField, "-minAnomalousScore", minAnomScoreDefault);
		addTextFieldIntParam(args, maxSizeTextField, "-maxsize", maxSizeDefault);
		addTextFieldIntParam(args, minSizeTextField, "-minsize", minSizeDefault);
		addTextFieldIntParam(args, nSubsTextField, "-nsubs", nSubsDefault);
		addTextFieldIntParam(args, outputTextField, "-output", outputDefault);
		appendCheckBoxParam(args, overlapCheckBox, "-overlap");
		appendCheckBoxParam(args, pruneCheckBox, "-prune");
		appendCheckBoxParam(args, undirectedCheckBox, "-undirected");
		appendCheckBoxParam(args, valuebasedCheckBox, "-valuebased");
		
		return args;
	}
	
	// handles the advaced/simple, restore defaults, and start gbad buttons
	public void actionPerformed(ActionEvent e)
	{
		if(e.getActionCommand().equals(advancedText))
		{
			advanced.setActionCommand(simpleText);
			advanced.setText(simpleText);
			
			hideParameters(false);
		}
		else if(e.getActionCommand().equals(simpleText))
		{
			advanced.setActionCommand(advancedText);
			advanced.setText(advancedText);
			
			hideParameters(true);
		}
		else if(e.getActionCommand().equals(restoreDefaultsText))
		{
			restoreDefaults();
		}
		else if(e.getActionCommand().equals(startGBADText))
		{
			ActionEvent newEvent = new ActionEvent(this, 0, startGBADText);
			fireActoinEvent(newEvent);
		}
		else
		{
			return;
		}
		
		validate();
		repaint();
	}
		
	public void addActionListener(ActionListener l)
	{
		listenerList.add(ActionListener.class, l);
	}
	
	public void removeActionListener(ActionListener l)
	{
		listenerList.remove(ActionListener.class, l);
	}
	
	protected void fireActoinEvent(ActionEvent e)
	{
		Object[] listeners = listenerList.getListenerList();
		for (int i = listeners.length-2; i>=0; i-=2)
		{
			if (listeners[i]==ActionListener.class)
			{
				((ActionListener)listeners[i+1]).actionPerformed(e);
			}
		}
	}
}

// class to force right alignment in JComboBoxes
class ListCellRenderer extends DefaultListCellRenderer
{
	public Component getListCellRendererComponent(JList list, Object value, int index, boolean isSelected, boolean cellHasFocus)
	{
		JLabel lbl = (JLabel)super.getListCellRendererComponent(list, value, index, isSelected, cellHasFocus);
		lbl.setHorizontalAlignment(SwingConstants.RIGHT);
		return lbl;
	}
}

// hides/shows the parameter (if any) associated with the selected anomaly detection algorithm 
class StateChangeListener implements ItemListener
{
	private JLabel mdlLabel;
	private JTextField mdlTextField;
	
	private JLabel mpsLabel;
	private JTextField mpsTextField;
	
	private JLabel probLabel;
	private JTextField probTextField;
	
	public StateChangeListener(JLabel mdlLabel, JTextField mdlTextField, JLabel mpsLabel, JTextField mpsTextField, JLabel probLabel, JTextField probTextField)
	{
		this.mdlLabel = mdlLabel;
		this.mdlTextField = mdlTextField;
		this.mpsLabel = mpsLabel;
		this.mpsTextField = mpsTextField;
		this.probLabel = probLabel;
		this.probTextField = probTextField;
	}
	
	public void itemStateChanged(ItemEvent evt)
	{
		String item = (String)evt.getItem();
		
		if(item.equals("-mdl"))
		{
			mdlLabel.setVisible(true);
			mdlTextField.setVisible(true);
			
			mpsLabel.setVisible(false);
			mpsTextField.setVisible(false);
			
			probLabel.setVisible(false);
			probTextField.setVisible(false);
		}
		else if(item.equals("-mps"))
		{
			mdlLabel.setVisible(false);
			mdlTextField.setVisible(false);
			
			mpsLabel.setVisible(true);
			mpsTextField.setVisible(true);
			
			probLabel.setVisible(false);
			probTextField.setVisible(false);
		}
		else if(item.equals("-prob"))
		{
			mdlLabel.setVisible(false);
			mdlTextField.setVisible(false);
			
			mpsLabel.setVisible(false);
			mpsTextField.setVisible(false);
			
			probLabel.setVisible(true);
			probTextField.setVisible(true);
		}
		else
		{
			mdlLabel.setVisible(false);
			mdlTextField.setVisible(false);
			
			mpsLabel.setVisible(false);
			mpsTextField.setVisible(false);
			
			probLabel.setVisible(false);
			probTextField.setVisible(false);
		}
	}
}

/*
Key listener on a JTextField.  The source of any KeyEvent is assumed
to be a JTextField.  If the text field does not contain a positive
integer, the text field is colored to show the invalid value.
*/
class PostiveIntegerKeyListener implements KeyListener
{
	private String toolTipText;
	
	public PostiveIntegerKeyListener(String toolTipText)
	{
		this.toolTipText = toolTipText;
	}
	
	public void keyPressed(KeyEvent e)
	{
	
	}
	
	public void keyReleased(KeyEvent e)
	{
		JTextField textField = (JTextField)e.getSource();
		
		String num = textField.getText();
		
		if(!num.equals(""))
		{
			try
			{
				int value = Integer.parseInt(num);
				
				if(value < 0)
				{
					throw new NumberFormatException("Value must be an positive integer.");
				}
				
				textField.setBackground(new Color(255, 255, 255));
				textField.setToolTipText(toolTipText);
			}
			catch(NumberFormatException nfe)
			{
				textField.setBackground(new Color(255, 255, 190));
				textField.setToolTipText("Value must be a positive integer.");
			}
		}
	}
		
	public void keyTyped(KeyEvent e)
	{
		
	}
}

/*
Key listener on a JTextField.  The source of any KeyEvent is assumed
to be a JTextField.  If the text field does not contain a value between
0.0 and 1.0, the text field is colored to show the invalid value.
*/
class PercentKeyListener implements KeyListener
{
	private String toolTipText;
	
	public PercentKeyListener(String toolTipText)
	{
		this.toolTipText = toolTipText;
	}
	
	public void keyPressed(KeyEvent e)
	{
	
	}
	
	public void keyReleased(KeyEvent e)
	{
		JTextField textField = (JTextField)e.getSource();
		
		String num = textField.getText();
			
		if(!num.equals(""))
		{
			try
			{
				double value = Double.parseDouble(num);
				
				if((value < 0.0) || (value > 1.0))
				{
					throw new NumberFormatException("Value must be between 0.0 and 1.0.");
				}
				
				textField.setBackground(new Color(255, 255, 255));
				textField.setToolTipText(toolTipText);
			}
			catch(NumberFormatException nfe)
			{
				textField.setBackground(new Color(255, 255, 190));
				textField.setToolTipText("Value must be between 0.0 and 1.0.");
			}
		}
	}
		
	public void keyTyped(KeyEvent e)
	{
		
	}
}

/*
Key listener on a JTextField.  The source of any KeyEvent is assumed
to be a JTextField.  If the text field does not contain an integer from
1 to 5, the text field is colored to show the invalid value.
*/
class OneToFiveKeyListener implements KeyListener
{
	private String toolTipText;
	
	public OneToFiveKeyListener(String toolTipText)
	{
		this.toolTipText = toolTipText;
	}
	
	public void keyPressed(KeyEvent e)
	{
	
	}
	
	public void keyReleased(KeyEvent e)
	{
		JTextField textField = (JTextField)e.getSource();
		
		String num = textField.getText();
			
		if(!num.equals(""))
		{
			try
			{
				int value = Integer.parseInt(num);
				
				if((value < 0) || (value > 5))
				{
					throw new NumberFormatException("Value must be an integer between 0 and 5.");
				}
				
				textField.setBackground(new Color(255, 255, 255));
				textField.setToolTipText(toolTipText);
			}
			catch(NumberFormatException nfe)
			{
				textField.setBackground(new Color(255, 255, 190));
				textField.setToolTipText("Value must be an integer between 0 and 5.");
			}
		}
	}
		
	public void keyTyped(KeyEvent e)
	{
		
	}
}

/*
Key listener on a JTextField.  The source of any KeyEvent is assumed
to be a JTextField.  If the text field does not contain an integer
greater than 1, the text field is colored to show the invalid value.
*/
class GreaterThanOneKeyListener implements KeyListener
{
	private String toolTipText;
	
	public GreaterThanOneKeyListener(String toolTipText)
	{
		this.toolTipText = toolTipText;
	}
	
	public void keyPressed(KeyEvent e)
	{
	
	}
	
	public void keyReleased(KeyEvent e)
	{
		JTextField textField = (JTextField)e.getSource();
		
		String num = textField.getText();
		
		if(!num.equals(""))
		{
			try
			{
				int value = Integer.parseInt(num);
				
				if(value < 2)
				{
					throw new NumberFormatException("Value must be an integer greater than 1.");
				}
				
				textField.setBackground(new Color(255, 255, 255));
				textField.setToolTipText(toolTipText);
			}
			catch(NumberFormatException nfe)
			{
				textField.setBackground(new Color(255, 255, 190));
				textField.setToolTipText("Value must be an integer greater than 1.");
			}
		}
	}
		
	public void keyTyped(KeyEvent e)
	{
		
	}
}

/*
Key listener on a JTextField.  The source of any KeyEvent is assumed
to be a JTextField.  If the text field does not contain a positive
integer, the text field is colored to show the invalid value.
*/
class LimitKeyListener implements KeyListener
{
	private String toolTipText;
	
	public LimitKeyListener(String toolTipText)
	{
		this.toolTipText = toolTipText;
	}
	
	public void keyPressed(KeyEvent e)
	{
	
	}
	
	public void keyReleased(KeyEvent e)
	{
		JTextField textField = (JTextField)e.getSource();
		
		String num = textField.getText();
		
		if(!num.equals("( |E(G)| / 2 )"))
		{
			try
			{
				int value = Integer.parseInt(num);
				
				if(value < 0)
				{
					throw new NumberFormatException("Value must be an positive integer.");
				}
				
				textField.setBackground(new Color(255, 255, 255));
				textField.setToolTipText(toolTipText);
			}
			catch(NumberFormatException nfe)
			{
				textField.setBackground(new Color(255, 255, 190));
				textField.setToolTipText("Value must be an integer.");
			}
		}
		else
		{
			textField.setBackground(new Color(255, 255, 255));
			textField.setToolTipText(toolTipText);
		}
	}
		
	public void keyTyped(KeyEvent e)
	{
		
	}
}

/*
Key listener on a JTextField.  The source of any KeyEvent is assumed
to be a JTextField.  If the text field does not contain a double
greater than 0, the text field is colored to show the invalid value.
*/
class MaxKeyListener implements KeyListener
{
	private String toolTipText;
	
	public MaxKeyListener(String toolTipText)
	{
		this.toolTipText = toolTipText;
	}
	
	public void keyPressed(KeyEvent e)
	{
	
	}
	
	public void keyReleased(KeyEvent e)
	{
		JTextField textField = (JTextField)e.getSource();
		
		String num = textField.getText();
		
		if(!num.equals("MAX"))
		{
			try
			{
				double value = Double.parseDouble(num);
				
				if(value < 0)
				{
					throw new NumberFormatException("Value must be greater than or equal to 0.0.");
				}
				
				textField.setBackground(new Color(255, 255, 255));
				textField.setToolTipText(toolTipText);
			}
			catch(NumberFormatException nfe)
			{
				textField.setBackground(new Color(255, 255, 190));
				textField.setToolTipText("Value must be greater than or equal to 0.0.");
			}
		}
		else
		{
			textField.setBackground(new Color(255, 255, 255));
			textField.setToolTipText(toolTipText);
		}
	}
		
	public void keyTyped(KeyEvent e)
	{
		
	}
}

/*
Key listener on a JTextField.  The source of any KeyEvent is assumed
to be a JTextField.  If the text field does not contain a double
greater than 0, the text field is colored to show the invalid value.
*/
class MinKeyListener implements KeyListener
{
	private String toolTipText;
	
	public MinKeyListener(String toolTipText)
	{
		this.toolTipText = toolTipText;
	}
	
	public void keyPressed(KeyEvent e)
	{
	
	}
	
	public void keyReleased(KeyEvent e)
	{
		JTextField textField = (JTextField)e.getSource();
		
		String num = textField.getText();
		
		if(!num.equals("MIN"))
		{
			System.out.println("num is [" + num + "]");
			try
			{
				double value = Double.parseDouble(num);
				
				if(value < 0)
				{
					throw new NumberFormatException("Value must be greater than or equal to 0.0.");
				}
				
				textField.setBackground(new Color(255, 255, 255));
				textField.setToolTipText(toolTipText);
			}
			catch(NumberFormatException nfe)
			{
				textField.setBackground(new Color(255, 255, 190));
				textField.setToolTipText("Value must be greater than or equal to 0.0.");
			}
		}
		else
		{
			textField.setBackground(new Color(255, 255, 255));
			textField.setToolTipText(toolTipText);
		}
	}
		
	public void keyTyped(KeyEvent e)
	{
		
	}
}