tree grammar jcf_walker;

options
{ 
        language = C; 
        tokenVocab = jcf;
        ASTLabelType = pANTLR3_BASE_TREE;
}

@header
{
        #include "../jcf_walker_impl.h"
}
