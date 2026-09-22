
// GameViewerDoc.h : interface of the CGameViewerDoc class
//


#pragma once


class CGameViewerDoc : public CDocument
{
protected: // create from serialization only
	CGameViewerDoc();
	DECLARE_DYNCREATE(CGameViewerDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CGameViewerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};


