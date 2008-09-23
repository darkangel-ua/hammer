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

project :        rules;
rules : (rule { hammer_delete_rule_result($rule.result); })*;

rule returns[void* result] 
@init { void * args_list = hammer_make_args_list(PARSER->super); }
: ^(RULE_CALL ID args[args_list]*) { result = hammer_rule_call(PARSER->super, $ID.text->chars, args_list); }; 

args[void* args_list] : string_list[args_list]  
	              | feature_set_arg[args_list]
		      | null_arg[args_list] 
		      | string_arg[args_list]
		      | feature_arg[args_list]
		      | requirements { hammer_add_arg_to_args_list(args_list, hammer_make_requirements_decl_arg($requirements.result)); }
		      | project_requirements { hammer_add_arg_to_args_list(args_list, hammer_make_project_requirements_decl_arg($project_requirements.result)); } 
		      | sources_decl { hammer_add_arg_to_args_list(args_list, hammer_make_sources_decl_arg($sources_decl.sources)); };

feature_set_arg[void* args_list] : ^(FEATURE_SET_ARG feature_set) { hammer_add_arg_to_args_list(args_list, hammer_make_feature_set_arg($feature_set.fs)); };
feature_arg[void* args_list] : ^(FEATURE_ARG feature) { hammer_add_arg_to_args_list(args_list, hammer_make_feature_arg($feature.feature)); };
string_arg[void* args_list] : ^(STRING_ARG ID) { hammer_add_string_arg_to_args_list(PARSER->super, args_list, $ID.text->chars); };
string_list[void* args_list]
@init{ void* arg = hammer_make_string_list(); }
        : ^(STRING_LIST string_list_id[arg]+) { hammer_add_arg_to_args_list(args_list, arg); }; 
                
string_list_id[void* list]
        : ID { hammer_add_id_to_string_list(PARSER->super, list, $ID.text->chars); };
        
null_arg[void* args_list]
@init { void* arg = hammer_make_null_arg(); }
        : NULL_ARG { hammer_add_arg_to_args_list(args_list, arg); };            
project_requirements returns[void* result] 
	: ^(PROJECT_REQUIREMENTS ID requirements) {result = hammer_make_project_requirements_decl($ID.text->chars, $requirements.result); };
requirements returns[void* result] 
@init { result = hammer_make_requirements_decl(); }
	: ^(REQUIREMENTS_DECL (conditional_features { hammer_add_conditional_to_rdecl($conditional_features.c, result); } | 
	                       feature { hammer_add_feature_to_rdecl($feature.feature, result); })+
	   );
conditional_features returns[void* c] 
@init { c = hammer_make_requirements_condition(); }
	: ^(CONDITIONAL_FEATURES condition[c] COLON feature { hammer_set_condition_result(c, $feature.feature); });
condition[void* c]
	: ^(CONDITION (feature { hammer_add_feature_to_condition($feature.feature, c); })+);
feature returns[void* feature]
	: ^(FEATURE feature_name=ID feature_value=ID) { feature = hammer_create_feature(PARSER->super, $feature_name.text->chars, $feature_value.text->chars); };
sources_decl returns[void* sources]
@init { sources = hammer_make_sources_decl(); } 
//	: ^(SOURCES_DECL (ID { hammer_add_source_to_sources_decl(PARSER->super, $ID.text->chars, sources); } )+) 
//	| ^(SOURCES_DECL rule) { hammer_add_rule_result_to_source_decl($rule.result, sources); hammer_delete_rule_result($rule.result); }
	:  ^(SOURCES_DECL (source_decl { hammer_add_source_to_sources_decl(sources, $source_decl.sd); } | sources_decl_rule_invoke[sources])+);

source_decl returns[void* sd]
@init { sd = hammer_make_source_decl(PARSER->super); }	
	: ^(SOURCE_DECL target_path { hammer_source_decl_set_target_path(PARSER->super, sd, $target_path.tp); } 
	                target_name[sd] 
	                target_features[sd]);
target_path returns[void* tp] 
@init { tp = hammer_make_target_path(); }
        : ^(TARGET_PATH (ID { hammer_add_to_target_path(PARSER->super, tp, $ID); } | SLASH { hammer_add_to_target_path(PARSER->super, tp, $SLASH); })+);
target_name[void* sd] : ^(TARGET_NAME ID { hammer_source_decl_set_target_name(PARSER->super, sd, $ID.text->chars); } )
                      | ^(TARGET_NAME NULL_ARG { hammer_source_decl_set_target_name(PARSER->super, sd, NULL); } );
target_features[void* sd] : feature_set { hammer_source_decl_set_target_properties(sd, $feature_set.fs);} 
            		  | ^(FEATURE_SET NULL_ARG);
feature_set returns[void* fs] 
@init { fs = hammer_make_feature_set(PARSER->super); }
 			: ^(FEATURE_SET (feature { hammer_add_feature_to_feature_set(fs, $feature.feature); } )+); 
 			
sources_decl_rule_invoke[void* sources] : rule { hammer_add_rule_result_to_source_decl($rule.result, sources); hammer_delete_rule_result($rule.result); };
