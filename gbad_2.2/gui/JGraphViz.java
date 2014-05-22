import java.util.*;

import java.awt.*;
import java.awt.image.*;

import java.io.*;
import javax.swing.*;
import javax.imageio.*;

public class JGraphViz
{

	/*
	dot draws directed graphs.  It works well on DAGs and other graphs that
	can  be  drawn  as  hierarchies.   It  reads attributed graph files and
	writes drawings.
	*/
	private String DOT = "dot";
	
	/*
	neato  draws  undirected graphs using ``spring'' models (see Kamada and
	Kawai, Information Processing Letters 31:1, April 1989).   Input  files
	must  be  formatted  in the dot attributed graph language.
	*/
	private String NEATO = "neato";
	
	/*
	twopi  draws  graphs  using a radial layout (see G. Wills, Symposium on
	Graph Drawing GD'97, September, 1997).  Basically, one node  is  chosen
	as the center and put at the origin.  The remaining nodes are placed on
	a sequence of concentric circles centered  about  the  origin,  each  a
	fixed  radial  distance from the previous circle.  All nodes distance 1
	from the center are placed on the first circle; all  nodes  distance  1
	from a node on the first circle are placed on the second circle; and so
	forth.
	*/
	private String TWOPI = "twopi";
	
	/*
	circo draws graphs using a circular layout (see Six and Tollis, GD  '99
	and  ALENEX  '99, and Kaufmann and Wiese, GD '02.)  The tool identifies
	biconnected components and draws the nodes of the component on  a  cir-
	cle.  The block-cutpoint tree is then laid out using a recursive radial
	algorithm. Edge crossings within a circle are minimized by  placing  as
	many  edges  on  the circle's perimeter as possible.  In particular, if
	the component is outerplanar, the component will have a planar  layout.
	*/
	private String CIRCO = "circo";
	
	/*
	fdp  draws  undirected  graphs using a ``spring'' model. It relies on a
	force-directed approach in the spirit of Fruchterman and Reingold  (cf.
	Software-Practice & Experience 21(11), 1991, pp. 1129-1164).
	*/
	private String FDP = "fdp";
	
	/*
	sfdp also draws undirected graphs using the ``spring'' model  described
	above,  but  it uses a multi-scale approach to produce layouts of large
	graphs in a reasonably short time.
	*/
	private String SFDP = "sfdp";
	
	private boolean usePreferences;
	
	// assume programs are in the path and no preference file is used
	public JGraphViz()
	{
		usePreferences = false;
	}
	
	// assume programs are in the path and preferenceFile is the preference file is used
	public JGraphViz(String preferenceFile) throws FileNotFoundException, IOException, SecurityException
	{
		usePreferences = true;
		
		String value;
		boolean change = false;
		
		/*
		For each program check if their is a property for it.  If a property is found use
		it, otherwise set as being in the path.
		*/
		
		value = System.getProperty(DOT);
		if(value == null)
		{
			System.setProperty(DOT, DOT);
			change = true;
		}
		
		value = System.getProperty(NEATO);
		if(value == null)
		{
			System.setProperty(NEATO, NEATO);
			change = true;
		}
		
		value = System.getProperty(TWOPI);
		if(value == null)
		{
			System.setProperty(TWOPI, TWOPI);
			change = true;
		}
		
		value = System.getProperty(CIRCO);
		if(value == null)
		{
			System.setProperty(CIRCO, CIRCO);
			change = true;
		}
		
		value = System.getProperty(FDP);
		if(value == null)
		{
			System.setProperty(FDP, FDP);
			change = true;
		}
		
		value = System.getProperty(SFDP);
		if(value == null)
		{
			System.setProperty(SFDP, SFDP);
			change = true;
		}
		
		// if any properties had to be written save them in preferenceFile
		if(change)
		{
			System.getProperties().store(new FileOutputStream(preferenceFile), null);
		}
	}
	
	// programs are in the spedified location and no preference file is used
	public JGraphViz(String dot, String neato, String twopi, String circo, String fdp, String sfdp)
	{
		usePreferences = false;
		
		this.DOT = dot;
		this.NEATO = neato;
		this.TWOPI = twopi;
		this.CIRCO = circo;
		this.FDP = fdp;
		this.SFDP = sfdp;
	}
	
	// programs are in the spedified location and preferenceFile is the preference file is used
	public JGraphViz(String preferenceFile, String dot, String neato, String twopi, String circo, String fdp, String sfdp) throws FileNotFoundException, IOException, SecurityException, NullPointerException, IllegalArgumentException
	{
		usePreferences = true;
		
		String value;
		boolean change = false;
		
		/*
		For each program check if their is a property for it.  If a property is found check if it
		equals the specified one.  If it is not, set it.
		*/
		
		value = System.getProperty(this.DOT);
		if((value == null) || (!value.equals(DOT)))
		{
			System.setProperty(this.DOT, DOT);
			change = true;
		}
		
		value = System.getProperty(this.NEATO);
		if((value == null) || (!value.equals(NEATO)))
		{
			System.setProperty(this.NEATO, NEATO);
			change = true;
		}
		
		value = System.getProperty(this.TWOPI);
		if((value == null) || (!value.equals(TWOPI)))
		{
			System.setProperty(this.TWOPI, TWOPI);
			change = true;
		}
		
		value = System.getProperty(this.CIRCO);
		if((value == null) || (!value.equals(CIRCO)))
		{
			System.setProperty(this.CIRCO, CIRCO);
			change = true;
		}
		
		value = System.getProperty(this.FDP);
		if((value == null) || (!value.equals(FDP)))
		{
			System.setProperty(this.FDP, FDP);
			change = true;
		}
		
		value = System.getProperty(this.SFDP);
		if((value == null) || (!value.equals(SFDP)))
		{
			System.setProperty(this.SFDP, SFDP);
			change = true;
		}
		
		// if any properties had to be written save them in preferenceFile
		if(change)
		{
			System.getProperties().store(new FileOutputStream(preferenceFile), null);
		}
	}
	
	public ImageIcon getDotImage(String graphFileName) throws FileNotFoundException, IOException, InterruptedException
	{
		return getDotImage(new File(graphFileName));
	}
	
	public ImageIcon getDotImage(File graphFile) throws FileNotFoundException, IOException, InterruptedException
	{
		return getImage(graphFile, ".dot", DOT);
	}
	
	public ImageIcon getNeatoImage(String graphFileName) throws FileNotFoundException, IOException, InterruptedException
	{
		return getNeatoImage(new File(graphFileName));
	}
	
	public ImageIcon getNeatoImage(File graphFile) throws FileNotFoundException, IOException, InterruptedException
	{
		return getImage(graphFile, ".neato", NEATO);
	}
	
	public ImageIcon getTwopiImage(String graphFileName) throws FileNotFoundException, IOException, InterruptedException
	{
		return getTwopiImage(new File(graphFileName));
	}
	
	public ImageIcon getTwopiImage(File graphFile) throws FileNotFoundException, IOException, InterruptedException
	{
		return getImage(graphFile, ".twopi", TWOPI);
	}
	
	public ImageIcon getCircoImage(String graphFileName) throws FileNotFoundException, IOException, InterruptedException
	{
		return getCircoImage(new File(graphFileName));
	}
	
	public ImageIcon getCircoImage(File graphFile) throws FileNotFoundException, IOException, InterruptedException
	{
		return getImage(graphFile, ".circo", CIRCO);
	}
	
	public ImageIcon getFdpImage(String graphFileName) throws FileNotFoundException, IOException, InterruptedException
	{
		return getFdpImage(new File(graphFileName));
	}
	
	public ImageIcon getFdpImage(File graphFile) throws FileNotFoundException, IOException, InterruptedException
	{
		return getImage(graphFile, ".fdp", FDP);
	}
	
	public ImageIcon getSfdpImage(String graphFileName) throws FileNotFoundException, IOException, InterruptedException
	{
		return getSfdpImage(new File(graphFileName));
	}
	
	public ImageIcon getSfdpImage(File graphFile) throws FileNotFoundException, IOException, InterruptedException
	{
		return getImage(graphFile, ".sfdp", SFDP);
	}
	
	private ImageIcon getImage(File graphFile, String append, String program) throws FileNotFoundException, IOException, InterruptedException
	{
		
		if(usePreferences)
		{
			try
			{
				String value = System.getProperty(program);
				if(value != null)
				{
					program = value;
				}
			}
			catch(Exception e)
			{
				// any of these errors should have been caught by the constructor
			}
		}
		
		// check if the dot file exists
		if(!graphFile.exists())
		{
			throw new FileNotFoundException("File Not Found: " + graphFile.toString());
		}
		
		// write the image file into a image dir
		String parent = graphFile.getParent();
		String fileName;
		
		if(parent == null)
		{
			fileName = "./images/";
		}
		else
		{
			fileName = parent + "/images/";
		}
		
		// make any dirs that do not exist
		(new File(fileName)).mkdirs();
		
		fileName += graphFile.getName() + append + ".png";
		
		if(!(new File(fileName)).exists())
		{
			// create the cmd vector to run the conversion program
			Vector<String> cmd = new Vector<String>();
			cmd.add(program);
			cmd.add("-Tpng");
			cmd.add(graphFile.toString());
			cmd.add("-o" + fileName);
			
			for(int i=0; i<cmd.size(); i++)
			{
				System.out.print(cmd.elementAt(i) + " ");
			}
			System.out.println("");
			
			// run the conversion program
			ProcessBuilder processBuilder = new ProcessBuilder(cmd);
			Process process;
			
			try
			{
				process = processBuilder.start();
			}
			catch(IOException ioe)
			{
				throw new IOException("Visualization Program (" + program + ") Not Found.");
			}
			
			// wait for it to complete
			process.waitFor();
			
			// check if their was an error
			BufferedReader reader = new BufferedReader(new InputStreamReader(process.getErrorStream()));
			
			StringBuffer error = new StringBuffer();
			String line = reader.readLine();
			
			while(line != null)
			{
				error.append(line);
				error.append("\n");
				line = reader.readLine();
			}
			
			// if an error occured in conversion throw it
			String errorMessage = error.toString();
			if(!errorMessage.equals(""))
			{
				throw new IOException(errorMessage);
			}
		}
		
		//BufferedImage image = ImageIO.read(new File(fileName));
		//return new ImageIcon(image);
		
		ImageIcon image = new ImageIcon(fileName);
		
		if(image.getImageLoadStatus() != MediaTracker.COMPLETE)
		{
			throw new IOException("Error reading image.");
		}
		
		return image;
	}
	
	public static void main(String[] args) throws Exception
	{
		JGraphViz gv = new JGraphViz();
		
		try
		{
			gv.getCircoImage("graph.dot");
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
}