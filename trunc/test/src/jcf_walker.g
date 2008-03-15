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

jcf_file : targets ;
targets : ^(TARGETS target+);
target 	: ^(TARGET attributes* target*);

attributes : ^(ATTRIBUTES attribute+); 
attribute 
	: type 
	| features ;
	
type 	: ^(TYPE_ATTR ID);	
features : ^(FEATURES_ATTR feature+);
feature  : ^(FEATURE ID ID);
