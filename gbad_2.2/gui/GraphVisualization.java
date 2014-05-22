import java.util.Timer;
import java.util.TimerTask;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;

import javax.imageio.ImageIO;

import javax.swing.BorderFactory;
import javax.swing.ImageIcon;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JSlider;

import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JProgressBar;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Image;
import java.awt.Insets;
import java.awt.MediaTracker;
import java.awt.Point;

import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.WindowEvent;
import java.awt.event.WindowFocusListener;

/*
This class is responsible for drawing the graphs.
*/
public class GraphVisualization extends JPanel implements ChangeListener, ItemListener
{
	public static final String dot = "Directed (dot)";
	public static final String neato = "Spring (neato)";
	public static final String twogpi = "Radial (twogpi)";
	public static final String circo = "Circular (circo)";
	public static final String fdp = "Spring (fdp)";
	public static final String sfdp = "LargeSpring (sfdp)";
	
	// the dot file currently being displayed
	private String fileName;
	
	// the image for the current dot file
	private ImageIcon originalImage;
	
	// class used to convert dot files into graphs
	private JGraphViz graphViz;
	
	// used creating and loaded images to free up the gui event thread
	private Timer timer;
	
	private JLabel graphLabel;
	
	// a modified JScrollPane to display the graph images
	private DraggableJScrollPane scrollPane;
	
	private JSlider zoom;
	private int curScale;
	
	private JComboBox layoutStyles;
	private final String[] layouts = {dot, neato, twogpi, circo, fdp, sfdp};
	
	private JComboBox imageScales;
	private final String[] stringScales = {"fast", "smooth", /*"averaging", "replicate"*/};
	private final int[] valueScales = {Image.SCALE_FAST, Image.SCALE_SMOOTH, /*Image.SCALE_AREA_AVERAGING, Image.SCALE_REPLICATE*/};
	
	public GraphVisualization()
	{
		super();
		
		fileName = null;
		originalImage = null;
		
		curScale = 100;
		
		/*
		Try to load the property file.  If it exists and can be loaded create
		a JGraphViz object with the given exe locations.  If the property file
		can not be loaded, assume exe files are in the path
		*/
		try
		{
			String file = System.getProperty("property_file");
			if(file == null)
			{
				throw new FileNotFoundException("Unable to determine preference file location.");
			}
			
			graphViz = new JGraphViz(file);
		}
		catch(FileNotFoundException fnfe)
		{
			graphViz = new JGraphViz();
			JOptionPane.showMessageDialog(this, fnfe.getMessage(), "Preference File Not Found", JOptionPane.ERROR_MESSAGE);
		}
		catch(IOException ioe)
		{
			graphViz = new JGraphViz();
			JOptionPane.showMessageDialog(this, ioe.getMessage(), "Error Writing Preference File", JOptionPane.ERROR_MESSAGE);
		}
		catch(SecurityException se)
		{
			graphViz = new JGraphViz();
			JOptionPane.showMessageDialog(this, "Unable to load graphviz exectuable locations from preferences.", "Security Exception", JOptionPane.ERROR_MESSAGE);
		}
		
		timer = new Timer();
		
		addContent();
	}
	
	private void addContent()
	{
		// SOUTH
		//zoom = new JSlider(new SliderModel());
		zoom = new JSlider(0, 100, 100);
		zoom.setMajorTickSpacing(25);
		zoom.setMinorTickSpacing(5);
		zoom.setSnapToTicks(true);
		zoom.addChangeListener(this);
		zoom.setPaintTicks(true);
		zoom.setToolTipText("Zoom");
		
		layoutStyles = new JComboBox(layouts);
		layoutStyles.addItemListener(this);
		layoutStyles.setToolTipText("Graph Layout Algorithm");
		
		imageScales = new JComboBox(stringScales);
		imageScales.addItemListener(this);
		imageScales.setToolTipText("Image Scaling Algorithm");
		
		JPanel westPanel = new JPanel();
		GridBagLayout gridbag = new GridBagLayout();
		westPanel.setLayout(gridbag);
		
		GridBagConstraints c = new GridBagConstraints();
		//c.insets = new Insets(0,5,5,5);
		
		c.anchor = GridBagConstraints.WEST;
		c.fill = GridBagConstraints.NONE;
		c.weightx = 1.0;
		gridbag.setConstraints(layoutStyles, c);
		westPanel.add(layoutStyles);
		
		c.anchor = GridBagConstraints.CENTER;
		westPanel.add(imageScales);
		
		c.anchor = GridBagConstraints.EAST;
		c.gridwidth = GridBagConstraints.REMAINDER;
		c.gridheight = GridBagConstraints.REMAINDER;
		gridbag.setConstraints(zoom, c);
		westPanel.add(zoom);
		
		// CENTER
		graphLabel = new JLabel("No Graph File Selected");
		scrollPane = new DraggableJScrollPane(graphLabel);
		scrollPane.setBorder(BorderFactory.createLoweredBevelBorder());
		scrollPane.setBackground(Color.white);
		
		setLayout(new BorderLayout());
		add(scrollPane, BorderLayout.CENTER);
		add(westPanel, BorderLayout.SOUTH);
	}
	
	public void open(File file)
	{
		open(file.toString());
	}
	
	public void open(String fileName)
	{
		this.fileName = fileName;
		itemStateChanged(new ItemEvent(layoutStyles, 0, layoutStyles.getSelectedItem(), ItemEvent.SELECTED));
	}
	
	public void close()
	{
		graphLabel.setText("");
		graphLabel.setIcon(null);
	}
	
	/*
	This function is called by a CreateGraphTask when an ImgaeIcon
	is has be read from disk.
	*/
	protected void notifyGraph(ImageIcon image)
	{
		originalImage = image;
		
		graphLabel.setIcon(originalImage);
		graphLabel.setText("");
		
		validate();
		repaint();
		
		Container container = this;
		while(container.getParent() != null)
		{
			container = container.getParent();
		}
		container.setEnabled(true);
		container.setFocusable(true);
	}
	
		/*
	This function is called by a CreateGraphTask when an ImgaeIcon
	failed to be loaded.
	
	t is the error that occursed when trying to render/load the image.
	*/
	protected void notifyGraphError(Throwable t)
	{
		Container container = this;
		while(container.getParent() != null)
		{
			container = container.getParent();
		}
		container.setEnabled(true);
		container.setFocusable(true);
		
		originalImage = null;
		graphLabel.setIcon(null);
		graphLabel.setText("Error: " + t.getMessage());
	}
	
	protected void notifyResize(ImageIcon image)
	{
		if(image.getImageLoadStatus() == MediaTracker.COMPLETE)
		{
			graphLabel.setIcon(image);
			graphLabel.setText("");
		}
		
		Container container = this;
		while(container.getParent() != null)
		{
			container = container.getParent();
		}
		container.setEnabled(true);
		container.setFocusable(true);
	}
	
	public void itemStateChanged(ItemEvent e)
	{
		if((e.getStateChange() == ItemEvent.SELECTED) && (fileName != null))
		{
			if(e.getSource() == layoutStyles)
			{
				//getValueIsAdjusting() 
				String style = e.getItem().toString();
				
				originalImage = null;
				graphLabel.setIcon(null);
				
				// its a good idea to try to garbage collect before creating the new image
				System.gc();
				
				CreateGraphTask task = new CreateGraphTask(style, fileName, graphViz, this);
				
				Container container = this;
				while(container.getParent() != null)
				{
					container = container.getParent();
				}
				container.setEnabled(false);
				container.setFocusable(false);
				
				timer.schedule(task, 0);
			}
			else if(e.getSource() == imageScales)
			{
				curScale = zoom.getValue();
				//System.out.println(curScale);
				
				System.gc();
				
				int scaleType = valueScales[imageScales.getSelectedIndex()];
				ResizeGraphTask task = new ResizeGraphTask(curScale, scaleType, originalImage, this);
				
				Container container = this;
				while(container.getParent() != null)
				{
					container = container.getParent();
				}
				container.setEnabled(false);
				container.setFocusable(false);
				
				timer.schedule(task, 0);
			}
		}
	}
	
	public void stateChanged(ChangeEvent e)
	{
		if((e.getSource() == zoom) && !zoom.getValueIsAdjusting() && (originalImage != null))
		{
			if(curScale != zoom.getValue())
			{
				curScale = zoom.getValue();
				//System.out.println(curScale);
				
				System.gc();
				
				int scaleType = valueScales[imageScales.getSelectedIndex()];
				ResizeGraphTask task = new ResizeGraphTask(curScale, scaleType, originalImage, this);
				
				Container container = this;
				while(container.getParent() != null)
				{
					container = container.getParent();
				}
				container.setEnabled(false);
				container.setFocusable(false);
				
				timer.schedule(task, 0);
				
				/*
				if(curScale < 100)
				{
					double scale;
					
					if(curScale == 0)
					{
						scale = 2 / 100.0;
					}
					else
					{
						scale = curScale / 100.0;
					}
					
					double width = originalImage.getIconWidth() * scale;
					double height = originalImage.getIconHeight() * scale;
					
					if((width > 0.0) && (height > 0.0))
					{
						System.gc();
						
						ImageIcon scaledIcon = new ImageIcon(originalImage.getImage().getScaledInstance((int)width, (int)height, Image.SCALE_FAST));
						//graphLabel.setIcon(scaledIcon);
						
						notifyResize(scaledIcon);
					}
				}
				else
				{
					graphLabel.setIcon(originalImage);
				}
				*/
			}
		}
	}
}

class CreateGraphTask extends TimerTask
{
	String style;
	String fileName;
	JGraphViz graphViz;
	GraphVisualization gv;
	
	public CreateGraphTask(String style, String fileName, JGraphViz graphViz, GraphVisualization gv)
	{
		this.style = style;
		this.fileName = fileName;
		this.graphViz = graphViz;
		this.gv = gv;
	}
	
	public void run()
	{
		JProgressBar progressBar = new JProgressBar(0,1);
		progressBar.setPreferredSize(new Dimension(175,20));
		progressBar.setValue(0);
		progressBar.setString("Creating Graph File");
		progressBar.setIndeterminate(true);
		progressBar.setStringPainted(true);
		
		JLabel label = new JLabel("Generating Graph Image");
		
		JPanel center_panel = new JPanel();
		center_panel.setLayout(new BorderLayout());
		center_panel.add(label, BorderLayout.NORTH);
		center_panel.add(progressBar, BorderLayout.SOUTH);
		
		// this is a hack
		// this will not work unless gv has a parent and the parent is a jframe
		Container container = gv;
		while(container.getParent() != null)
		{
			container = container.getParent();
		}
		
		final JDialog dialog = new JDialog((JFrame)container, "Loading. . .");
		dialog.setDefaultCloseOperation(JDialog.DO_NOTHING_ON_CLOSE);
		
		Point p = container.getLocation();
		p.translate(container.getWidth()/2, container.getHeight()/2);
		
		dialog.setLocation(p);
		dialog.setResizable(false);
		dialog.getContentPane().add(center_panel, BorderLayout.CENTER);
		dialog.pack();
		dialog.setVisible(true);
		dialog.toFront();
		
		try
		{
			ImageIcon image;
			
			// its a good idea to try to garbage collect before creating the new image
			System.gc();
			
			if(style.equals(GraphVisualization.neato))
			{
				image = graphViz.getNeatoImage(fileName);
			}
			else if(style.equals(GraphVisualization.twogpi))
			{
				image = graphViz.getTwopiImage(fileName);
			}
			else if(style.equals(GraphVisualization.circo))
			{
				image = graphViz.getCircoImage(fileName);
			}
			else if(style.equals(GraphVisualization.fdp))
			{
				image = graphViz.getFdpImage(fileName);
			}
			else if(style.equals(GraphVisualization.sfdp))
			{
				image = graphViz.getSfdpImage(fileName);
			}
			else
			{
				image = graphViz.getDotImage(fileName);
			}
			
			progressBar.setValue(1);
			progressBar.setString("Opening File");
			
			gv.notifyGraph(image);
			
			dialog.setVisible(false);
		}
		catch(Throwable t)
		{
			progressBar.setValue(1);
			progressBar.setString("Opening File");
			dialog.setVisible(false);
			System.out.println("done");
			
			gv.notifyGraphError(t);
		}
	}
}

class ResizeGraphTask extends TimerTask
{
	int curScale;
	int scaleType;
	ImageIcon originalImage;
	GraphVisualization gv;
	
	public ResizeGraphTask(int curScale, int scaleType, ImageIcon originalImage, GraphVisualization gv)
	{
		this.curScale = curScale;
		this.scaleType = scaleType;
		this.originalImage = originalImage;
		this.gv = gv;
	}
	
	public void run()
	{
		JProgressBar progressBar = new JProgressBar(0,1);
		progressBar.setPreferredSize(new Dimension(175,20));
		progressBar.setValue(0);
		progressBar.setString("Resizing Graph");
		progressBar.setIndeterminate(true);
		progressBar.setStringPainted(true);
		
		JLabel label = new JLabel("Generating Graph Image");
		
		JPanel center_panel = new JPanel();
		center_panel.setLayout(new BorderLayout());
		center_panel.add(label, BorderLayout.NORTH);
		center_panel.add(progressBar, BorderLayout.SOUTH);
		
		// this is a hack
		// this will not work unless gv has a parent and the parent is a jframe
		Container container = gv;
		while(container.getParent() != null)
		{
			container = container.getParent();
		}
		
		final JDialog dialog = new JDialog((JFrame)container, "Scaling. . .");
		dialog.setDefaultCloseOperation(JDialog.DO_NOTHING_ON_CLOSE);
		
		Point p = container.getLocation();
		p.translate(container.getWidth()/2, container.getHeight()/2);
		
		dialog.setLocation(p);
		dialog.setResizable(false);
		dialog.getContentPane().add(center_panel, BorderLayout.CENTER);
		dialog.pack();
		dialog.setVisible(true);
		dialog.toFront();
		
		if(curScale < 100)
		{
			double scale;
			
			if(curScale == 0)
			{
				scale = 2 / 100.0;
			}
			else
			{
				scale = curScale / 100.0;
			}
			
			double width = originalImage.getIconWidth() * scale;
			double height = originalImage.getIconHeight() * scale;
			
			if((width > 0.0) && (height > 0.0))
			{
				System.gc();
				
				ImageIcon scaledIcon = new ImageIcon(originalImage.getImage().getScaledInstance((int)width, (int)height, scaleType));
				//graphLabel.setIcon(scaledIcon);
				
				gv.notifyResize(scaledIcon);
			}
		}
		else
		{
			gv.notifyResize(originalImage);
		}
		
		dialog.setVisible(false);
	}
}
