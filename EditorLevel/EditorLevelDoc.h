
// EditorLevelDoc.h : interface of the CEditorLevelDoc class
//


#pragma once


class CEditorLevelDoc : public CDocument
{
protected: // create from serialization only
	CEditorLevelDoc();
	DECLARE_DYNCREATE(CEditorLevelDoc)

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
	virtual ~CEditorLevelDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};


