
// NewClippingDlg.cpp : implementation file
//The name of this program is Program for calculating the intersection of two polygons approximating two circles with cut sectors

#include "stdafx.h"
#include "NewClipping.h"
#include "NewClippingDlg.h"
#include "afxdialogex.h"
#include "clipper.hpp" //Thanks a lot to Angus Johnson for his free Clipper Library from www.angusj.com/delphi/clipper.php
#include <gdiplusgraphics.h>
#include "gdiplusgraphics.h"
#include <math.h>
#include <fstream>
#include <gdiplus.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#define PI 3.1415926535
#endif

//I don't use boolean variables because I prefer int (0/1)
// CNewClippingDlg dialog

using namespace ClipperLib;

CNewClippingDlg::CNewClippingDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_NEWCLIPPING_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CNewClippingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER1, SectorSlider);
	DDX_Control(pDX, IDC_SLIDER2, XSlider);
	DDX_Control(pDX, IDC_SLIDER3, YSlider);
	DDX_Control(pDX, IDC_SLIDER4, RadiusSlider);
	DDX_Control(pDX, IDC_SLIDER5, AngleSlider);
	DDX_Control(pDX, IDC_CHECK1, CheckSave);
	DDX_Control(pDX, IDC_EDIT7, FirstCts);
	DDX_Control(pDX, IDC_EDIT8, SecondCds);
	DDX_Control(pDX, IDC_CHECK2, DrawStart);
	DDX_Control(pDX, IDC_CHECK3, CheckDrawSaving);
	DDX_Control(pDX, IDC_EDIT1, CutSizeText);
	DDX_Control(pDX, IDC_RADIO2, CounterClock);
	DDX_Control(pDX, IDC_RADIO1, ClockLike);
	DDX_Control(pDX, IDC_EDIT2, BeforeSize);
	DDX_Control(pDX, IDC_COMBO1, ListOfLines);
	DDX_Control(pDX, IDC_EDIT3, CurrentX);
	DDX_Control(pDX, IDC_EDIT6, CurrentY);
	DDX_Control(pDX, IDC_EDIT9, CurrentSector);
	DDX_Control(pDX, IDC_EDIT5, CurrentRadius);
	DDX_Control(pDX, IDC_EDIT4, CurrentAngle);
	DDX_Control(pDX, IDC_SLIDER6, SliderChooser);
	DDX_Control(pDX, IDC_COMBO2, ChosenMode);
	DDX_Control(pDX, IDC_SLIDER7, AddSizeSlider);
}

BEGIN_MESSAGE_MAP(CNewClippingDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CNewClippingDlg::AddMirrorLine)
	ON_BN_CLICKED(IDC_BUTTON2, &CNewClippingDlg::Activate)
	ON_WM_LBUTTONDOWN()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON3, &CNewClippingDlg::SaveCoordinates)
	ON_BN_CLICKED(IDC_CHECK1, &CNewClippingDlg::IfChecking)
	ON_BN_CLICKED(IDC_RADIO2, &CNewClippingDlg::ConterRadio)
	ON_BN_CLICKED(IDC_RADIO1, &CNewClippingDlg::ToRadio)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CNewClippingDlg::ChooseStrictLine)
	ON_BN_CLICKED(IDC_BUTTON4, &CNewClippingDlg::SaveFullFigure)
END_MESSAGE_MAP()


// CNewClippingDlg message handlers

BOOL CNewClippingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.


int X1, Y1, X2, Y2, CenterX, CenterY, CenterSmallX, CenterSmallY; //Coordintaes of the mirror line and main area center and center of area where the saved figure is presented
//StatusButton(0/1) and CounterClicks - to create the custom mirror line; DrawCircles and DrawSolution - to permit to draw (to avoid an assert error)
//NumberOfFile - to count the files while Autosave is on; CON - the counter of numbers of clicks on tick "Autosave" (to correct message about this function)
int StatusLButton = 0, CounterClicks = 0, DrawCircles = 0, DrawSolution = 0, NumberOfFile = 1, CON = 0;
double knt; //to consider the quadrant to create the correct round (NB: the orientation must be Clockwise)
long double idouble, dX1, dY1, dX2, dY2, k, X0, Y0, Angle/*, PI = 3.1415926535*/; //To correct calculate of sin and cos we need a float type. Define doesn't work in Release mode
RECT WindowRect; //Structure of the program window
CRect DrawRect, DrawSavingRect; //1 - Structure of main area 2 - Area where what will be saved is drawn
std::vector<Path> RealSector(1); //Array of points of sector which we control
std::vector<Path> MirrorSector(1); //Array of points of reflected sector
std::vector<Path> SolutionSector(2); //The result of intersection two previous sectors
std::vector<Path> SendForSaving(1); //The shortened array of solution sector. We don't need many useless points 
POINT *RealSectorStruct; //
POINT *MirrorSectorStruct; // 
POINT **SolutionStruct; //  
POINT *DrawWhatInSave; // The same for drawing
POINT *FormRealCircle(int XSlider, int YSlider, int AngleSlider, int SectorSlider, int RadiusSlider); //This function create a circle sector
POINT **SolutionCircle(std::vector<Path> SubjectSector, std::vector<Path> ClipperSector); //The main function in the program. It makes an intersection
CString TextForLine, FileName, DirectPathName, ForNumber; //to format in the correct string type
ofstream file;
void SaveCoordinatesFunction(); //It works. I know nothing:)
int neededsize = 0, WriteHappend = 0;
int CustomStep = 0; //To correct shorten of solution sector
int OrientationState = 1; //(0/1) To claim the orientation of solution sector points
int ChosenVar = 5; //Each chosenvar-s point will be saved
long double newCenterx, newCentery;  //Coordinates of the center of blue figure (what will be saved)

void CNewClippingDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		CWnd* m_hWnd = AfxGetApp()->GetMainWnd();
		m_hWnd->MoveWindow(0, 0, 100, 100);

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CPaintDC dc(this);
		CPen PenForSolution(PS_SOLID, 1, RGB(255, 0, 0)), p1(PS_SOLID, 1, RGB(0, 255, 0)), 
					PenForCut(PS_SOLID, 1, RGB(0, 0, 255));
		if (CenterX != 0 && CenterY != 0) {
			dc.Rectangle(DrawRect); //Main area
			dc.Rectangle(DrawSavingRect); //Sub-main area
		}
		if (X1 != 0 && Y1 != 0 && X2 != 0 && Y2 != 0) {
			dc.MoveTo(X1, Y1);
			dc.LineTo(X2, Y2);
		}
		if (DrawCircles == 1) {
			dc.Polygon(RealSectorStruct, 362 - SectorSlider.GetPos());
			switch (ChosenMode.GetCurSel()) {
			case 0: dc.Polygon(MirrorSectorStruct, 362 - SectorSlider.GetPos()); break;
			case 1: dc.Polygon(MirrorSectorStruct, 362); break;
			case 2: dc.Polygon(MirrorSectorStruct, 4); break;
			}		
		}
		if (DrawSolution != 0) {
			dc.SelectObject(PenForSolution);
			for (int i = 0; i < DrawSolution; i++) {
				dc.Polygon(SolutionStruct[i], SolutionSector[i].size());
				if (DrawStart.GetCheck() == 1) {
					dc.Ellipse(SolutionStruct[i][0].x - 3, SolutionStruct[i][0].y - 3, SolutionStruct[i][0].x + 3, SolutionStruct[i][0].y + 3);
				}
			}
		}
		if (SendForSaving[0].size() > 0 && CheckDrawSaving.GetCheck() == 1){
			dc.SelectObject(PenForCut);
			dc.Polygon(DrawWhatInSave, SendForSaving[0].size());
		}
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CNewClippingDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CNewClippingDlg::AddMirrorLine()
{
	StatusLButton = 1;
}

//There is we activate the program: draw the borders of areas, positions of sliders, add combo-boxes
void CNewClippingDlg::Activate()
{
	GetClientRect(&WindowRect);
	CenterX = ((WindowRect.left + WindowRect.right) / 2) + 80;
	CenterY = (WindowRect.top + WindowRect.bottom) / 2;
	CenterSmallX = CenterX - 557 - XSlider.GetPos();
	CenterSmallY = CenterY - YSlider.GetPos();
	DrawRect.left = CenterX - 330;
	DrawRect.top = CenterY - 330;
	DrawRect.right = CenterX + 330;
	DrawRect.bottom = CenterY + 330;
	DrawSavingRect.left = CenterX - 737;
	DrawSavingRect.top = CenterY - 181;
	DrawSavingRect.right = CenterX - 376;
	DrawSavingRect.bottom = CenterY + 181;
	XSlider.SetRange(5, 445, 1);
	XSlider.SetPos(240);
	YSlider.SetRange(5, 445, 1);
	YSlider.SetPos(240);
	AngleSlider.SetRange(0, 360, 1);
	AngleSlider.SetPos(0);
	RadiusSlider.SetRange(10, 180, 1);
	RadiusSlider.SetPos(180);
	SectorSlider.SetRange(0, 360, 1);
	SectorSlider.SetPos(90);
	SliderChooser.SetRange(0, 1, 1);
	SliderChooser.SetPos(0);
	CheckDrawSaving.SetCheck(1);
	ClockLike.SetCheck(1);

	AddSizeSlider.SetRange(0, 200, 1);
	AddSizeSlider.SetPos(0);

	ListOfLines.AddString(_T("1. Vertical"));
	ListOfLines.AddString(_T("2. Horizontal"));
	ListOfLines.AddString(_T("3. Diagonal+"));
	ListOfLines.AddString(_T("4. Diagonal-"));

	ChosenMode.AddString(_T("1. Sector"));
	ChosenMode.AddString(_T("2. Full Circle"));
	ChosenMode.AddString(_T("3. Rectangle"));

	ChosenMode.SetCurSel(1);

	InvalidateRect(DrawRect, 1);
	InvalidateRect(DrawSavingRect, 1);
	OnPaint();
}

//To draw the custom mirror line we count a number of clicks
void CNewClippingDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	int Coords[4];
	if (StatusLButton == 1) {
		CounterClicks++;
		if (CounterClicks == 1) {
			Coords[0] = point.x;
			Coords[1] = point.y;
			X1 = Coords[0];
			Y1 = Coords[1];
		}
	}
	if (StatusLButton == 1 && CounterClicks == 2) {
		Coords[2] = point.x;
		Coords[3] = point.y;
		X2 = Coords[2];
		Y2 = Coords[3];
		StatusLButton = 0;
		CounterClicks = 0;
		DrawCircles = 1;
		DrawNow();
		FillingForms();
		FillingSimpleForms();
	}

	CDialogEx::OnLButtonDown(nFlags, point);
}

POINT *FormRealCircle(int XSlider, int YSlider, int AngleSlider, int SectorSlider, int RadiusSlider) {
	if (DrawCircles == 1) {
		RealSectorStruct = new POINT[362 - SectorSlider];
		RealSector[0].clear(); //A new sector each time
		for (int i = 0 + AngleSlider; i < 362 + AngleSlider - SectorSlider; i++) {
			idouble = (long double)i;
			RealSector[0] << IntPoint((CenterX + XSlider + RadiusSlider * cos(idouble*(PI / 180)))*pow(10.0, 6.0),
				(CenterY + YSlider + RadiusSlider * sin(idouble*(PI / 180)))*pow(10.0, 6.0)); //We multiply by 1 000 000 to increase the quality of count significantly
			RealSectorStruct[i - AngleSlider].x = CenterX + XSlider + RadiusSlider * cos(idouble*(PI / 180));
			RealSectorStruct[i - AngleSlider].y = CenterY + YSlider + RadiusSlider * sin(idouble*(PI / 180));
		}
		RealSector[0] << IntPoint((CenterX + XSlider)*pow(10.0, 6.0), (CenterY + YSlider)*pow(10.0, 6.0)); //The last point is always the center of the sector
		RealSectorStruct[362 - SectorSlider - 1].x = CenterX + XSlider;
		RealSectorStruct[362 - SectorSlider - 1].y = CenterY + YSlider;
		return RealSectorStruct;
	}
}

//There is we use special formulas to count the coordintaes for mirrored sector
POINT *CNewClippingDlg::FormMirrorCircle(int RadiusSlider, int XSlider, int YSlider, POINT *RealSectorStruct,
																			int SectorSlider, int AngleSlider, int AddSizeSlider) {
	long double k = 0, X0 = 0, Y0 = 0;
	dX1 = (long double)X1;
	dY1 = (long double)Y1;
	dX2 = (long double)X2;
	dY2 = (long double)Y2;
	MirrorSector[0].clear();
	switch (ChosenMode.GetCurSel()) {
	case 0: {
		MirrorSectorStruct = new POINT[362 - SectorSlider];
		for (int i = 0; i < 362 - SectorSlider; i++) {
			k = -1 * ((dX2 - dX1)*(dX1 - RealSectorStruct[i].x) + (dY2 - dY1)*(dY1 - RealSectorStruct[i].y)) /
				((dX2 - dX1)*(dX2 - RealSectorStruct[i].x) + (dY2 - dY1)*(dY2 - RealSectorStruct[i].y)); //special coefficient
			X0 = (dX1 + k * dX2) / (1 + k); //
			Y0 = (dY1 + k * dY2) / (1 + k); //exactely those formulas
			MirrorSector[0] << IntPoint(int(2 * X0 - RealSectorStruct[i].x)*pow(10.0, 6.0),
				int(2 * Y0 - RealSectorStruct[i].y)*pow(10.0, 6.0)); //exactely those formulas
			MirrorSectorStruct[i].x = int(2 * X0 - RealSectorStruct[i].x);
			MirrorSectorStruct[i].y = int(2 * Y0 - RealSectorStruct[i].y);
		}
	}; break;
	case 1: {
		MirrorSectorStruct = new POINT[362];
		k = -1 * ((dX2 - dX1)*(dX1 - (CenterX + XSlider)) + (dY2 - dY1)*(dY1 - (CenterY + YSlider))) /
			((dX2 - dX1)*(dX2 - (CenterX + XSlider)) + (dY2 - dY1)*(dY2 - (CenterY + YSlider)));
		newCenterx = 2 * ((dX1 + k * dX2) / (1 + k)) - (CenterX + XSlider);
		newCentery = 2 * ((dY1 + k * dY2) / (1 + k)) - (CenterY + YSlider);
		for (int i = 0; i < 362; i++) {
			idouble = (long double)i;
			MirrorSector[0] << IntPoint(int(newCenterx + ((RadiusSlider + AddSizeSlider) * cos(idouble*(PI / 180))))*pow(10.0, 6.0),
				int(newCentery + ((RadiusSlider + AddSizeSlider) * sin(idouble*(PI / 180))))*pow(10.0, 6.0));
			MirrorSectorStruct[i].x = int(newCenterx + ((RadiusSlider + AddSizeSlider) * cos(idouble * (PI / 180))));
			MirrorSectorStruct[i].y = int(newCentery + ((RadiusSlider + AddSizeSlider) * sin(idouble * (PI / 180))));
		}
	}; break;
	//There is the square is created. It hasn't worked propertly with Angle Slider 
	case 2: {
		Angle = (double)AngleSlider;
		MirrorSectorStruct = new POINT[4];
		k = -1 * ((dX2 - dX1)*(dX1 - (CenterX + XSlider)) + (dY2 - dY1)*(dY1 - (CenterY + YSlider))) /
			((dX2 - dX1)*(dX2 - (CenterX + XSlider)) + (dY2 - dY1)*(dY2 - (CenterY + YSlider)));
		newCenterx = 2 * ((dX1 + k * dX2) / (1 + k)) - (CenterX + XSlider);
		newCentery = 2 * ((dY1 + k * dY2) / (1 + k)) - (CenterY + YSlider);
		MirrorSector[0] << IntPoint(int(newCenterx + ((RadiusSlider + AddSizeSlider) * cos((PI + Angle) / 4))) * pow(10.0, 6.0),
										int(newCentery + ((RadiusSlider + AddSizeSlider) * sin((PI + Angle) / 4))) * pow(10.0, 6.0));
		MirrorSectorStruct[0].x = int(newCenterx + ((RadiusSlider + AddSizeSlider) * cos((PI + Angle) / 4)));
		MirrorSectorStruct[0].y = int(newCentery + ((RadiusSlider + AddSizeSlider) * sin((PI + Angle) / 4)));
		MirrorSector[0] << IntPoint(int(newCenterx + ((RadiusSlider + AddSizeSlider) * cos((3* (PI + Angle)) / 4))) * pow(10.0, 6.0),
										int(newCentery + ((RadiusSlider + AddSizeSlider) * sin((3 * (PI + Angle)) / 4))) * pow(10.0, 6.0));
		MirrorSectorStruct[1].x = int(newCenterx + ((RadiusSlider + AddSizeSlider) * cos((3 * (PI + Angle)) / 4)));
		MirrorSectorStruct[1].y = int(newCentery + ((RadiusSlider + AddSizeSlider) * sin((3 * (PI + Angle)) / 4)));
		MirrorSector[0] << IntPoint(int(newCenterx + ((RadiusSlider + AddSizeSlider) * cos((5 * (PI + Angle)) / 4))) * pow(10.0, 6.0),
										int(newCentery + ((RadiusSlider + AddSizeSlider) * sin((5 * (PI + Angle)) / 4))) * pow(10.0, 6.0));
		MirrorSectorStruct[2].x = int(newCenterx + ((RadiusSlider + AddSizeSlider) * cos((5 * (PI + Angle)) / 4)));
		MirrorSectorStruct[2].y = int(newCentery + ((RadiusSlider + AddSizeSlider) * sin((5 * (PI + Angle)) / 4)));
		MirrorSector[0] << IntPoint(int(newCenterx + ((RadiusSlider + AddSizeSlider) * cos((7 * (PI + Angle)) / 4))) * pow(10.0, 6.0),
										int(newCentery + ((RadiusSlider + AddSizeSlider) * sin((7 * (PI + Angle)) / 4))) * pow(10.0, 6.0));
		MirrorSectorStruct[3].x = int(newCenterx + ((RadiusSlider + AddSizeSlider) * cos((7 * (PI + Angle)) / 4)));
		MirrorSectorStruct[3].y = int(newCentery + ((RadiusSlider + AddSizeSlider) * sin((7 * (PI + Angle)) / 4)));
	}; break;
	}
	return MirrorSectorStruct;
};

POINT **SolutionCircle(std::vector<Path> SubjectSector, std::vector<Path> ClipperSector) {
	SolutionSector.clear();
	Clipper DoClip; //
	DoClip.AddPath(SubjectSector[0], ptSubject, true); //
	DoClip.AddPath(ClipperSector[0], ptClip, true); //
	DoClip.Execute(ctIntersection, SolutionSector, pftNonZero, pftNonZero); //The clipper library syntax
	if (OrientationState == 1) {
		ReversePaths(SolutionSector);
	}
	if (SolutionSector.size() != 0) {
		SolutionStruct = new POINT*[SolutionSector.size()];
		for (int i = 0; i < SolutionSector.size(); i++) { //It can have 1 or 2 dimention (usually 1)
			SolutionStruct[i] = new POINT[SolutionSector[i].size()];
			for (int j = 0; j < SolutionSector[i].size(); j++) {
				SolutionStruct[i][j].x = SolutionSector[i][j].X / pow(10.0, 6.0); //
				SolutionStruct[i][j].y = SolutionSector[i][j].Y / pow(10.0, 6.0); //We divide by 1 000 000 to come back to native coordinates
			}
			DrawSolution = i + 1; //Let the program draw the results
		}
		return SolutionStruct;
	}
	else {
		DrawSolution = 0;
		return 0;
	}
}

//There is we check if any slider had its position changed
void CNewClippingDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CSliderCtrl *pSlider = reinterpret_cast<CSliderCtrl*>(pScrollBar);
	if (pSlider == &XSlider || pSlider == &YSlider || pSlider == &AngleSlider || pSlider == &SectorSlider || pSlider == &RadiusSlider || pSlider || pSlider == &SliderChooser) {
		if (CounterClock.GetCheck() == 1) {
			OrientationState = 1;
		}
		if (ClockLike.GetCheck() == 1) {
			OrientationState = 0;
		}
		CenterSmallX = CenterX - 557 - XSlider.GetPos() + 240;
		CenterSmallY = CenterY - YSlider.GetPos() + 240;
		DrawNow();
		if (CheckSave.GetCheck() == 1) {
			SaveCoordinatesFunction();
		}
		FillingForms();
	}
	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CNewClippingDlg::SaveCoordinates()
{
	if (SolutionSector.size() != 1) { //We save only one-dimension solution
		AfxMessageBox(_T("Секторы не пересекаются или пересечение не односвязно"), MB_ICONINFORMATION);
	}
	else {
		SaveCoordinatesFunction(); //It works as I wanted. I know nothing:)
	}
}

void SaveCoordinatesFunction() {
	if (SendForSaving.size() == 1) {
		if (DirectPathName == "") {
			CFileDialog SFile(false, _T(".txt"), _T("Example 1"), 0, _T("TXT Files (*.txt)|*.txt|"));
			SFile.DoModal();
			DirectPathName = SFile.GetFolderPath();
		}
		ForNumber.Format(_T("%d"), NumberOfFile);
		FileName = DirectPathName + _T("\\Example ") + ForNumber + _T(".txt"); //I still don't make a custom name of saved files
		file.open(FileName, ios::out);
		if (file.is_open()) {
			for (int i = 0; i < SendForSaving[0].size(); i++) {
				file << SendForSaving[0][i].X << " " << SendForSaving[0][i].Y << endl; //View like "25.256 45.212"
			}
			file.close();
		}
	}
}

POINT *CNewClippingDlg::FormStructForSave(POINT **SolutionStruct)
{
	//There is we decide what to choose: quality or speed
	if (SliderChooser.GetPos() == 0) {
		ChosenVar = 5; //speed (default)
	}
	else {
		ChosenVar = 2; //quality
	}
	if (SolutionSector.size() != 0) {
		knt = -1 * ((Y2 - Y1) / (X2 - X1 + 0.1)); //+0.1 to avoid dividing by zero
		neededsize = -1; //to start counting from 0
		WriteHappend = 0; //Remember that we wrote down the dot
		SendForSaving[0].clear();
		if (SolutionSector[0].size() > 50) {
			CustomStep = trunc(SolutionSector[0].size() / 50) + ChosenVar; //We calculate how many dots we need
		}
		else {
			CustomStep = 1;
		}
		DrawWhatInSave = new POINT[SolutionSector[0].size()]; //I still don't know why we must declare exactely such range. New array will be less than solution sector. Isn't it? 
		for (int j = 0; j < SolutionSector[0].size(); j++) {
			WriteHappend = 0;
			//We need to consider and save the "Key-dots" to keep the correct form of figure
			//Key-dots tn this case are dots wuth distance between each other less then 4.
			//Such dots we write down anyway
			if (j > 0 && (abs(SolutionStruct[0][j].x - SolutionStruct[0][j - 1].x) > 4 || abs(SolutionStruct[0][j].y - SolutionStruct[0][j - 1].y) > 4)) {
				neededsize++;
				DrawWhatInSave[neededsize].x = (CenterSmallX - (CenterX - SolutionStruct[0][j].x));
				DrawWhatInSave[neededsize].y = (CenterSmallY - (CenterY - SolutionStruct[0][j].y));
				SendForSaving[0] << IntPoint((DrawWhatInSave[neededsize].x - CenterSmallX + CenterX) * 0.35,
					(DrawWhatInSave[neededsize].y - CenterSmallY + CenterY) * 0.35);
				WriteHappend = 1;
			}
			if (WriteHappend == 1) {
				continue;
			}
			//It may be useless. We round with special function (up or dowb) in each quandrant
			else {
				if (j % CustomStep == 0) {
					neededsize++;
					if (j <= (SolutionSector[0].size() / 2)) {
						if (knt >= 0) {
							DrawWhatInSave[neededsize].x = floor(CenterSmallX - (CenterX - SolutionStruct[0][j].x));
							DrawWhatInSave[neededsize].y = floor(CenterSmallY - (CenterY - SolutionStruct[0][j].y));
						}
						else {
							DrawWhatInSave[neededsize].x = floor(CenterSmallX - (CenterX - SolutionStruct[0][j].x));
							DrawWhatInSave[neededsize].y = ceil(CenterSmallY - (CenterY - SolutionStruct[0][j].y));
						}
					}
					else {
						if (knt >= 0) {
							DrawWhatInSave[neededsize].x = ceil(CenterSmallX - (CenterX - SolutionStruct[0][j].x));
							DrawWhatInSave[neededsize].y = ceil(CenterSmallY - (CenterY - SolutionStruct[0][j].y));
						}
						else {
							DrawWhatInSave[neededsize].x = ceil(CenterSmallX - (CenterX - SolutionStruct[0][j].x));
							DrawWhatInSave[neededsize].y = floor(CenterSmallY - (CenterY - SolutionStruct[0][j].y));
						}
					}
					SendForSaving[0] << IntPoint((DrawWhatInSave[neededsize].x - CenterSmallX + CenterX) * 0.35,
													(DrawWhatInSave[neededsize].y - CenterSmallY + CenterY) * 0.35);
				}
			}
		}
		return DrawWhatInSave;
	}
	else {
		return 0;
	}
}

void CNewClippingDlg::IfChecking()
{
	//The program remember the path to save for the first time and then save all the next files there
	if (DirectPathName == "") {
		CFileDialog SFile(false, _T(".txt"), _T("Example 1"), 0, _T("TXT Files (*.txt)|*.txt|"));
		SFile.DoModal();
		DirectPathName = SFile.GetFolderPath();
		SaveCoordinatesFunction();
	}
	if (CON % 2 == 0) {
		CON++;
		AfxMessageBox(_T("Включена функция автосохранения.\nПри каждои изменении любого из ползунка будет сохранен новый файл"), MB_ICONINFORMATION);
	}
	else {
		CON++;
		AfxMessageBox(_T("Функция автосохранения отключена"), MB_ICONWARNING);
	}
}


void CNewClippingDlg::ConterRadio()
{
	OrientationState = 1;
	AfxMessageBox(_T("Выбран обход против часовой стрелки (новый)"), MB_ICONINFORMATION);
	// TODO: Add your control notification handler code here
}


void CNewClippingDlg::ToRadio()
{
	OrientationState = 0;
	AfxMessageBox(_T("Выбран обход по часовой стрелке (как раньше)"), MB_ICONINFORMATION);
	// TODO: Add your control notification handler code here
}


void CNewClippingDlg::ChooseStrictLine()
{
	switch (ListOfLines.GetCurSel()) {
		case 0: {X1 = CenterX; Y1 = CenterY + 325; X2 = CenterX; Y2 = CenterY - 325; }; break;
		case 1: {X1 = CenterX - 325; Y1 = CenterY; X2 = CenterX + 325; Y2 = CenterY; }; break;
		case 2: {X1 = CenterX - 325; Y1 = CenterY + 325; X2 = CenterX + 325; Y2 = CenterY - 325; }; break;
		case 3: {X1 = CenterX + 325; Y1 = CenterY + 325; X2 = CenterX - 325; Y2 = CenterY - 325; }; break;
	}
	DrawCircles = 1;
	DrawNow();
	FillingForms();
	FillingSimpleForms();
	// TODO: Add your control notification handler code here
}

//The second main function that calls to form all the sectors and draws it
void CNewClippingDlg::DrawNow()
{
	RealSectorStruct = FormRealCircle(XSlider.GetPos() - 240, YSlider.GetPos() - 240, AngleSlider.GetPos(), SectorSlider.GetPos(), RadiusSlider.GetPos());
	MirrorSectorStruct = FormMirrorCircle(RadiusSlider.GetPos(), XSlider.GetPos() - 240, YSlider.GetPos() - 240, RealSectorStruct,
																		SectorSlider.GetPos(), AngleSlider.GetPos(), AddSizeSlider.GetPos());
	SolutionStruct = SolutionCircle(RealSector, MirrorSector);
	DrawWhatInSave = FormStructForSave(SolutionStruct);
	InvalidateRect(DrawRect, 1);
	InvalidateRect(DrawSavingRect, 1);
	OnPaint();
	delete[] RealSectorStruct;
	delete[] MirrorSectorStruct;
	delete[] SolutionStruct;
	delete[] DrawWhatInSave;
}

//The function that fillings almost all the edit controls
void CNewClippingDlg::FillingForms() {
	if (DrawSolution > 0) {
		TextForLine.Format(_T("%d"), SolutionSector[0].size());
		BeforeSize.SetWindowTextW(TextForLine);
		TextForLine.Format(_T("%d"), SendForSaving[0].size());
		CutSizeText.SetWindowTextW(TextForLine);
	}
	else {
		BeforeSize.SetWindowTextW(_T(""));
		CutSizeText.SetWindowTextW(_T(""));
	}
	TextForLine.Format(_T("%d"), XSlider.GetPos() - 240);
	CurrentX.SetWindowTextW(TextForLine);
	TextForLine.Format(_T("%d"), YSlider.GetPos() - 240);
	CurrentY.SetWindowTextW(TextForLine);
	TextForLine.Format(_T("%d"), 360 - SectorSlider.GetPos());
	CurrentSector.SetWindowTextW(TextForLine);
	TextForLine.Format(_T("%d"), RadiusSlider.GetPos());
	CurrentRadius.SetWindowTextW(TextForLine);
	TextForLine.Format(_T("%d"), -1 * AngleSlider.GetPos());
	CurrentAngle.SetWindowTextW(TextForLine);
}

//These edit controls may be empty and we describe them in another function
void CNewClippingDlg::FillingSimpleForms()
{
	TextForLine.Format(_T("(%d; %d)"), X1/2, Y1/2);
	FirstCts.SetWindowTextW(TextForLine);
	TextForLine.Format(_T("(%d; %d)"), X2/2, Y2/2);
	SecondCds.SetWindowTextW(TextForLine);
	TextForLine.Format(_T("%d"), SolutionSector[0].size());
	BeforeSize.SetWindowTextW(TextForLine);
	TextForLine.Format(_T("%d"), SendForSaving[0].size());
	CutSizeText.SetWindowTextW(TextForLine);
	XSlider.EnableWindow(true);
	YSlider.EnableWindow(true);
	AngleSlider.EnableWindow(true);
	RadiusSlider.EnableWindow(true);
	SectorSlider.EnableWindow(true);

	AddSizeSlider.EnableWindow(true);
}

void CNewClippingDlg::SaveFullFigure()
{
	if (DirectPathName == "") {
		CFileDialog SFile(false, _T(".txt"), _T("FullFigure 1"), 0, _T("TXT Files (*.txt)|*.txt|"));
		SFile.DoModal();
		DirectPathName = SFile.GetFolderPath();
	}
	ForNumber.Format(_T("%d"), NumberOfFile);
	FileName = DirectPathName + _T("\\FullFigure ") + ForNumber + _T(".txt");
	NumberOfFile++; //Auto-numbering of saved files
	file.open(FileName, ios::out);
	if (file.is_open()) {
		for (int i = 0; i < RealSector[0].size(); i++) {
			if (i == 0 || i == RealSector[0].size() - 1 || i == RealSector[0].size() - 2 || i % (RealSector[0].size() / 35) == 0) {
				file << (RealSector[0][i].X * 0.35) / pow(10.0, 6.0) << " " << (RealSector[0][i].Y * 0.35) / pow(10.0, 6.0) << endl;
			} //We multiply by 0.35 to reduce extent of coordinates because the following work is more optimal with small numbers
		}
		file.close();
	}
	// TODO: Add your control notification handler code here
}
