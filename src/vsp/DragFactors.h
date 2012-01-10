
#ifndef __DRAG_FACTORS_H
#define __DRAG_FACTORS_H

class DragFactors
{
public:

	DragFactors()
	{
		m_MinChord = 0.0;
		m_AvgChord = 0.0;
		m_MaxChord = 0.0;

		m_MinThickToChord = 0.0;
		m_AvgThickToChord = 0.0;
		m_MaxThickToChord = 0.0;

		m_AvgSweep = 0.0;

		m_Length = 0.0;
		m_MaxXSecArea = 0.0;
		m_LengthToDia = 0.0;
	}

	double m_MinChord;
	double m_AvgChord;
	double m_MaxChord;

	double m_MinThickToChord;
	double m_AvgThickToChord;
	double m_MaxThickToChord;
	
	double m_AvgSweep;

	double m_Length;
	double m_MaxXSecArea;
	double m_LengthToDia;

};


#endif
