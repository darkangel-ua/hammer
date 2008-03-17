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

jcf_file[void * t] : targets[t] ;
targets[void* t] : ^(TARGETS target[t, 1]+);
target[void* t, int is_top]
@init {void * tt = 0;} 	: ^(TARGET ID { tt = get_target($ID.text->chars, t, is_top); } attributes[tt]* target[tt, 0]*);

attributes[void* t] : ^(ATTRIBUTES attribute[t]+); 
attribute[void* t] 
	: type[t] 
	| features[t] ;
	
type[void* t] 	: ^(TYPE_ATTR ID);	
features[void* t] : ^(FEATURES_ATTR feature[t]+);
feature[void* t]  : ^(FEATURE ID ID);
