import java.awt.*;
import java.awt.event.*;

import javax.swing.*;

public class DraggableJScrollPane extends JScrollPane
{
	private Point lastLocation;
	
	protected boolean dragOn;
	
	public static void main(String[] args)
	{
		DraggableJScrollPane p = new DraggableJScrollPane(new JLabel(new ImageIcon("test.png")));
		
		JFrame f = new JFrame("test");
		f.setSize(300,300);
		f.add(p);
		f.setVisible(true);
	}
	
	public DraggableJScrollPane()
	{
		super();
		
		enableEvents(AWTEvent.MOUSE_MOTION_EVENT_MASK | AWTEvent.MOUSE_EVENT_MASK);
		dragOn = false;
		
		setCursor(new Cursor(Cursor.HAND_CURSOR));
	}
	
	public DraggableJScrollPane(Component view)
	{
		super(view);
		
		enableEvents(AWTEvent.MOUSE_MOTION_EVENT_MASK | AWTEvent.MOUSE_EVENT_MASK);
		dragOn = false;
		
		setCursor(new Cursor(Cursor.HAND_CURSOR));
	}
	
	public DraggableJScrollPane(Component view, int vsbPolicy, int hsbPolicy)
	{
		super(view, vsbPolicy, hsbPolicy);
		
		enableEvents(AWTEvent.MOUSE_MOTION_EVENT_MASK | AWTEvent.MOUSE_EVENT_MASK);
		dragOn = false;
		
		setCursor(new Cursor(Cursor.HAND_CURSOR));
	}
	
	public DraggableJScrollPane(int vsbPolicy, int hsbPolicy)
	{
		super(vsbPolicy, hsbPolicy);
		
		enableEvents(AWTEvent.MOUSE_MOTION_EVENT_MASK | AWTEvent.MOUSE_EVENT_MASK);
		dragOn = false;
		
		setCursor(new Cursor(Cursor.HAND_CURSOR));
	}
	
	protected void processMouseEvent(MouseEvent e)
	{
		switch(e.getID())
		{
			case MouseEvent.MOUSE_PRESSED:
			{
				setCursor(new Cursor(Cursor.MOVE_CURSOR));
				dragOn = true;
				
				lastLocation = new Point((int)e.getX(), (int)e.getY());
				
				break;
			}
			case MouseEvent.MOUSE_CLICKED:
			{
				break;
			}
			case MouseEvent.MOUSE_RELEASED:
			{
				setCursor(new Cursor(Cursor.HAND_CURSOR));
				dragOn = false;
				break;
			}
		}
	}
	
	protected void processMouseMotionEvent(MouseEvent e)
	{
		switch(e.getID())
		{
			case MouseEvent.MOUSE_DRAGGED:
			{
				if(dragOn)
				{
					JViewport view = getViewport();
					Point viewCorrner = view.getViewPosition();
					
					Point curLocation = e.getPoint();
					
					double cornerX = viewCorrner.getX();
					double cornerY = viewCorrner.getY();
					
					double x = lastLocation.getX() - curLocation.getX();
					double y = lastLocation.getY() - curLocation.getY();
					
					double maxX = view.getView().getWidth() - view.getWidth();
					double maxY = view.getView().getHeight() - view.getHeight();
					
					if(x + cornerX < 0)
					{
						x = 0 - cornerX;
					}
					else if(x + cornerX > maxX)
					{
						x = maxX - cornerX;
					}
					
					if(y + cornerY < 0)
					{
						y = 0 - cornerY;
					}
					else if(y + cornerY > maxY)
					{
						y = maxY - cornerY;
					}
					
					viewCorrner.translate((int)x, (int)y);
					view.setViewPosition(viewCorrner);
					
					lastLocation = curLocation;
				}
				
				break;
			}
		}
	}
}