import java.io.*;

public class Main
{
	public static void main(String[] args) throws Exception
	{
		GUI gui;
		
		if(args.length > 0)
		{
			gui = new GUI(args[0]);
		}
		else
		{
			gui = new GUI();
		}
		
		gui.setVisible(true);
		
		/*
		File lastDir = new File(".");
		System.out.println(lastDir.getAbsoluteFile() );
		*/
	}
}