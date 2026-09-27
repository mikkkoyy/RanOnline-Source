
// EditorSkinCharDoc.h : interface of the CEditorSkinCharDoc class
//


#pragma once


class CEditorSkinCharDoc : public CDocument
{
protected: // create from serialization only
	CEditorSkinCharDoc();
	DECLARE_DYNCREATE(CEditorSkinCharDoc)

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
	virtual ~CEditorSkinCharDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

};
