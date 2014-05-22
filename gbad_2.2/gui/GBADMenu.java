import java.io.*;

import javax.swing.*;
import javax.swing.event.*;
import javax.swing.border.*;

import java.awt.*;
import java.awt.event.*;

public class GBADMenu extends JMenuBar implements ActionListener
{
	public static final int NEW_COMMAND = 1;
	public static final int OPEN_GRAPH_COMMAND = 2;
	public static final int OPEN_RESULTS_COMMAND = 3;
	public static final int CLOSE_COMMAND = 4;
	public static final int SAVE_COMMAND = 5;
	public static final int SAVE_AS_COMMAND = 6;
	public static final int PREFERENCES_COMMAND = 7;
	
	private static final String fileString = "File";
	
	private static final String newString = "New Graph";
	private static final String openGraphString = "Open Graph";
	private static final String openResultsString = "Open Results";
	
	private static final String closeString = "Close";
	private static final String saveString = "Save Graph";
	private static final String saveAsString = "Save Graph As...";
	
	private static final String preferencesString = "Preferences";
	
	private int mask;
	private int shift_mask;
	
	EventListenerList listenerList;
	
	public GBADMenu()
	{
		super();
		
		listenerList = new EventListenerList();
		
		setMask();
		
		add(getFileMenu());
	}
	
	private void setMask()
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
			mask = ActionEvent.META_MASK;
			shift_mask = mask | InputEvent.SHIFT_MASK;
		}
		else
		{
			mask = ActionEvent.CTRL_MASK;
			shift_mask = mask | InputEvent.SHIFT_MASK;
		}
	}
	
	private JMenu getFileMenu()
	{
		/*
		JMenuItem newItem = new JMenuItem(newString, KeyEvent.VK_N);
		newItem.setActionCommand(newString);
		newItem.addActionListener(this);
		newItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_N, mask));
		*/
		
		JMenuItem openGraphItem = new JMenuItem(openGraphString, KeyEvent.VK_O);
		openGraphItem.setActionCommand(openGraphString);
		openGraphItem.addActionListener(this);
		openGraphItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_O, mask));
		
		JMenuItem openResultsItem = new JMenuItem(openResultsString, KeyEvent.VK_R);
		openResultsItem.setActionCommand(openResultsString);
		openResultsItem.addActionListener(this);
		openResultsItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_R, mask));
		
		JMenuItem closeItem = new JMenuItem(closeString, KeyEvent.VK_C);
		closeItem.setActionCommand(closeString);
		closeItem.addActionListener(this);
		closeItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_W, mask));
		
		/*
		JMenuItem saveItem = new JMenuItem(saveString, KeyEvent.VK_S);
		saveItem.setActionCommand(saveString);
		saveItem.addActionListener(this);
		saveItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_S, mask));
		
		JMenuItem saveAsItem = new JMenuItem(saveAsString, KeyEvent.VK_S);
		saveAsItem.setActionCommand(saveAsString);
		saveAsItem.addActionListener(this);
		saveAsItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_S, shift_mask));
		*/
		
		JMenuItem preferencesItem = new JMenuItem(preferencesString, KeyEvent.VK_P);
		preferencesItem.setActionCommand(preferencesString);
		preferencesItem.addActionListener(this);
		//preferencesItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_S, shift_mask));
		
		JMenu fileMenu = new JMenu(fileString);
		fileMenu.setMnemonic(KeyEvent.VK_F);
		//fileMenu.add(newItem);
		fileMenu.add(openGraphItem);
		fileMenu.add(openResultsItem);
		fileMenu.addSeparator();
		fileMenu.add(closeItem);
		//fileMenu.add(saveItem);
		//fileMenu.add(saveAsItem);
		fileMenu.addSeparator();
		fileMenu.add(preferencesItem);
		
		return fileMenu;
	}
	
	public void actionPerformed(ActionEvent e)
	{
		if(e.getActionCommand().equals(newString))
		{
			ActionEvent newAction = new ActionEvent(this, NEW_COMMAND, newString);
			fireActoinEvent(newAction);
		}
		else if(e.getActionCommand().equals(openGraphString))
		{
			ActionEvent newAction = new ActionEvent(this, OPEN_GRAPH_COMMAND, openGraphString);
			fireActoinEvent(newAction);
		}
		else if(e.getActionCommand().equals(openResultsString))
		{
			ActionEvent newAction = new ActionEvent(this, OPEN_RESULTS_COMMAND, openResultsString);
			fireActoinEvent(newAction);
		}
		else if(e.getActionCommand().equals(closeString))
		{
			ActionEvent newAction = new ActionEvent(this, CLOSE_COMMAND, closeString);
			fireActoinEvent(newAction);
		}
		else if(e.getActionCommand().equals(saveString))
		{
			ActionEvent newAction = new ActionEvent(this, SAVE_COMMAND, saveString);
			fireActoinEvent(newAction);
		}
		else if(e.getActionCommand().equals(saveAsString))
		{
			ActionEvent newAction = new ActionEvent(this, SAVE_AS_COMMAND, saveAsString);
			fireActoinEvent(newAction);
		}
		else if(e.getActionCommand().equals(preferencesString))
		{
			ActionEvent newAction = new ActionEvent(this, PREFERENCES_COMMAND, preferencesString);
			fireActoinEvent(newAction);
		}
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