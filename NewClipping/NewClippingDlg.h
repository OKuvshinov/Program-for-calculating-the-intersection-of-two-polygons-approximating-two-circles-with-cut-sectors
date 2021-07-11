
// NewClippingDlg.h : header file
//

#pragma once

// CNewClippingDlg dialog
class CNewClippingDlg : public CDialogEx
{
// Construction
public:
	CNewClippingDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_NEWCLIPPING_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void AddMirrorLine();
	afx_msg void Activate();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	CSliderCtrl SectorSlider;
	CSliderCtrl XSlider;
	CSliderCtrl YSlider;
	CSliderCtrl RadiusSlider;
	CSliderCtrl AngleSlider;
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void SaveCoordinates();
	CButton CheckSave;
	afx_msg void IfChecking();
	CEdit FirstCts;
	CEdit SecondCds;
	CButton DrawStart;
	CButton CheckDrawSaving;
	CEdit CutSizeText;
	CButton CounterClock;
	CButton ClockLike;
	afx_msg void ConterRadio();
	afx_msg void ToRadio();
	CEdit BeforeSize;
	CComboBox ListOfLines;
	afx_msg void ChooseStrictLine();
	void DrawNow();
	void FillingForms();
	void FillingSimpleForms();
	POINT *FormStructForSave(POINT **SolutionStruct);
	POINT *FormMirrorCircle(int RadiusSlider, int XSlider, int YSlider, POINT *RealSectorStruct, int SectorSlider, int AngleSlider, int AddSizeSlider);
	CEdit CurrentX;
	CEdit CurrentY;
	CEdit CurrentSector;
	CEdit CurrentRadius;
	CEdit CurrentAngle;
	afx_msg void ChooseQoD(NMHDR *pNMHDR, LRESULT *pResult);
	CSliderCtrl SliderChooser;
	CComboBox ChosenMode;
	afx_msg void SaveFullFigure();
	CSliderCtrl AddSizeSlider;
	afx_msg void eps_sloy();
	CEdit Area_eps_sloy;
};
