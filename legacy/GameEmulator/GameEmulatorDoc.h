
// GameEmulatorDoc.h : interface of the CGameEmulatorDoc class
//


#pragma once


class CGameEmulatorDoc : public CDocument
{
protected: // create from serialization only
	CGameEmulatorDoc();
	DECLARE_DYNCREATE(CGameEmulatorDoc)

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
	virtual ~CGameEmulatorDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

};
