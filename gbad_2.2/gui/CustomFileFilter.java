import javax.swing.filechooser.FileFilter;

import java.util.*;
import java.io.File;

class CustomFileFilter extends FileFilter
{
	private Vector<String> fileExtensions;
	private String fileExtensionDescription;
	
	boolean saveAs;
	
	public CustomFileFilter()
	{
		fileExtensions = new Vector<String>();
		fileExtensionDescription = "";
		saveAs = false;
	}
	
	public CustomFileFilter(boolean saveAs)
	{
		fileExtensions = new Vector<String>();
		fileExtensionDescription = "";
		this.saveAs = saveAs;
	}
	
	public void addExtension(String extension)
	{
		if(!fileExtensions.contains(extension))
		{
			fileExtensions.add(extension);
		}
	}
	
	public boolean accept(File file)
	{
		if(file.isDirectory())
		{
			return true;
		}
		else
		{
			if(!saveAs)
			{
				return fileExtensions.contains(getExtension(file));
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
	
	public void setDescription(String fileExtensionDescription)
	{
		this.fileExtensionDescription = new String(fileExtensionDescription);
	}
	
	private String getExtension(File file) 
	{
		String fileName = file.getName();
		
		int index = fileName.lastIndexOf('.');
		
		if((index > 0) && (index < fileName.length() - 1))
		{ 
			return fileName.substring(index+1);
		}
		
		return fileName;
	}
}