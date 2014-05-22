import javax.swing.filechooser.FileFilter;

import java.util.StringTokenizer;
import java.io.File;

class GraphFileFilter extends FileFilter
{
	private final String fileExtension = "g";
	private final String fileExtensionDescription = ".g (Graph File)";
	
	boolean saveAs;
	
	public GraphFileFilter()
	{
		saveAs = false;
	}
	
	public GraphFileFilter(boolean saveAs)
	{
		this.saveAs = saveAs;
	}
	
	public boolean accept(File f)
	{
		if(f.isDirectory())
		{
			return true;
		}
		else
		{
			if(!saveAs)
			{
				StringTokenizer tokenizer = new StringTokenizer(f.getName(), ".");
				
				String extension = "";
				
				while(tokenizer.hasMoreTokens())
				{
					extension = tokenizer.nextToken();
				}
				
				return extension.equalsIgnoreCase(fileExtension);
			}
			else
			{
				return false;
			}
		}
	}
	
	public String getDescription()
	{
		return fileExtensionDescription;
	}
}