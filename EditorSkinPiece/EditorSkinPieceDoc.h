
// EditorSkinPieceDoc.h : interface of the CEditorSkinPieceDoc class
//


#pragma once


class CEditorSkinPieceDoc : public CDocument
{
protected: // create from serialization only
	CEditorSkinPieceDoc();
	DECLARE_DYNCREATE(CEditorSkinPieceDoc)

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
	virtual ~CEditorSkinPieceDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};


