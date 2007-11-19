tree grammar hammer_walker;

options
{ 
	language = C; 
 	tokenVocab = hammer;
 	ASTLabelType = pANTLR3_BASE_TREE;
}

@header
{
	#include "../hammer_walker_impl.h"
}

project : ;

