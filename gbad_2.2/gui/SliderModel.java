import javax.swing.BoundedRangeModel;

import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.event.EventListenerList;

/*
SliderModel for use in the resizing of the displayed graph.
*/
class SliderModel implements BoundedRangeModel
{
	protected ChangeEvent changeEvent = null;
	
	// stores listeners to notify when the slider moves
	protected EventListenerList listenerList = new EventListenerList();

	protected int maximum = 100;
	protected int minimum = 1;
	protected int extent = 0;
	protected int value = 100;
	protected boolean isAdjusting = false;

	public SliderModel()
	{
	
	}

	public int getMaximum()
	{
		return maximum;
	}

	public void setMaximum(int newMaximum)
	{
		// max has to be greater than value + extent
		// extent can be thought of as the width of the slider bar
		if(newMaximum >= value+extent)
		{
			maximum = newMaximum;
			fireStateChanged();
		}
	}

	public int getMinimum()
	{
		return minimum;
	}

	public void setMinimum(int newMinimum)
	{
		// the new min has to be >= the current value
		if(value >= newMinimum)
		{
			minimum = newMinimum;
			fireStateChanged();
		}
	}

	public int getValue()
	{
		return value;
	}

	public void setValue(int newValue)
	{
		if((newValue >= minimum) && (maximum >= newValue+extent))
		{
			value = newValue;
			fireStateChanged();
		}
		else if((newValue < minimum) && (value != minimum))
		{
			value = minimum;
			fireStateChanged();
		}
		else if((maximum < newValue+extent) && (value != maximum))
		{
			value = maximum;
			fireStateChanged();
		}
	}

	public int getExtent()
	{
		return extent;
	}

	public void setExtent(int newExtent)
	{
		
	}

	public boolean getValueIsAdjusting()
	{
		return isAdjusting;
	}

	public void setValueIsAdjusting(boolean b)
	{
		isAdjusting = b;
	}

	public void setRangeProperties(int newValue, int newExtent, int newMin, int newMax, boolean newAdjusting)
	{
		if(newMax <= minimum)
		{
			newMax = minimum + 1;
		}
		
		if(newValue > newMax)
		{
			newValue = newMax;
		}

		boolean changeOccurred = false;
		if(newValue != value)
		{
			value = newValue;
			changeOccurred = true;
		}
		
		if(newMax != maximum)
		{
			maximum = newMax;
			changeOccurred = true;
		}
		
		if(newAdjusting != isAdjusting)
		{
			maximum = newMax;
			isAdjusting = newAdjusting;
			changeOccurred = true;
		}

		if(changeOccurred)
		{
			fireStateChanged();
		}
	}
	
	public void addChangeListener(ChangeListener l)
	{
		listenerList.add(ChangeListener.class, l);
	}

	public void removeChangeListener(ChangeListener l)
	{
		listenerList.remove(ChangeListener.class, l);
	}

	protected void fireStateChanged()
	{
		Object[] listeners = listenerList.getListenerList();
		for (int i = listeners.length - 2; i >= 0; i -= 2)
		{
			if(listeners[i] == ChangeListener.class)
			{
				if(changeEvent == null)
				{
					changeEvent = new ChangeEvent(this);
				}
				((ChangeListener) listeners[i + 1]).stateChanged(changeEvent);
			}
		}
	}
}
