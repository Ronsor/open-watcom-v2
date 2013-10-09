extern void        ObjInit( void );
extern void        ObjFini( void );
extern void        InitSegDefs( void );
extern void        DefSegment( segment_id id, seg_attr attr, const char *str, uint align, bool use_16 );
extern bool        HaveCodeSeg( void );
extern segment_id  AskCodeSeg( void );
extern segment_id  AskAltCodeSeg( void );
extern segment_id  AskBackSeg( void );
extern segment_id  AskOP( void );
extern segment_id  AskSegID( void *hdl, cg_class class );
extern bool        AskSegBlank( segment_id id );
extern segment_id  SetOP( segment_id seg );
extern void        FlushOP( segment_id id );
extern bool        NeedBaseSet( void );
extern offset      AskLocation( void );
extern long_offset AskBigLocation( void );
extern offset      AskMaxSize( void );
extern long_offset AskBigMaxSize( void );
extern void        SetLocation( offset loc );
extern void        SetBigLocation( long_offset loc );
extern void        OutLabel( code_lbl *label );
extern void        *InitPatch( void );
extern void        AbsPatch( abspatch_handle patch, offset lc );
extern void        TellObjNewProc( sym_handle proc );
extern void        IncLocation( offset by );
extern bool        AskNameROM( pointer h, cg_class c );
extern void        OutLineNum( cg_linenum line, bool label_line );
extern void        BackPtr( bck_info *bck, segment_id seg, offset plus, type_def *tipe );
extern void        BackPtrBase( bck_info *bck, segment_id seg );
extern void        BackBigOffset( bck_info *bck, segment_id seg, offset plus );
extern void        FEPtr( sym_handle sym, type_def *tipe, offset plus );
extern void        FEPtrBaseOffset( sym_handle sym,  offset plus );
extern void        FEPtrBase( sym_handle sym );
