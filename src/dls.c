/******************************************************************************************************************************/
/* ABLS-AGENT-LIBS/dls.c         Fonctions communes de gestion des plugins DLS                                               */
/* Projet Abls-Habitat                               Gestion d'habitat                                                        */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * dls.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2026 - Sebastien LEFEVRE
 *
 * ABLS-AGENT-LIBS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * ABLS-AGENT-LIBS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ABLS-AGENT-LIBS; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

/**************************************************** Prototypes de fonctions *************************************************/
 #include "abls-agent-libs.h"

/******************************************************************************************************************************/
/* Dls_create_agent_plugin: Créé automatiquement le plugin de l'agent en parametre                                            */
/* Entrées: L'agent en cours                                                                                                  */
/* Sortie: FALSE si pb, TRUE sinon                                                                                            */
/******************************************************************************************************************************/
 gboolean Dls_create_agent_plugin ( struct ABLS_AGENT *agent )
  { if (!agent)
     { Info( __func__, "dls", NULL, LOG_ERR, "Dls_create_agent_plugin called with NULL agent" );
       return(FALSE);
     }

    JsonNode *PluginNode = Json_create();
    if (!PluginNode)
      { Info( __func__, "dls", agent->agent_tech_id, LOG_ALERT, "Memory error while creating DLS pluginNode" );
       return(FALSE);
     }
    Json_add_string ( PluginNode, "tech_id", agent->agent_tech_id );
    Json_add_int    ( PluginNode, "syn_id", 2 );                                /* Raccroché au synoptique Système (pas HOME) */
    gchar *name = Json_get_string ( agent->api_config, "description" );
    Json_add_string ( PluginNode, "name", name );
    Json_add_string ( PluginNode, "shortname", name );
    gchar package[128];
    g_snprintf ( package, sizeof(package), "Agent_%s", agent->agent_classe );
    Json_add_string ( PluginNode, "package", package );

    JsonNode *api_result = Http_Post_to_global_API ( agent, "/run/dls/create", PluginNode );
    if (api_result == NULL || Json_get_int ( api_result, "http_code" ) != 200)
     { Info( __func__, "dls", agent->agent_tech_id, LOG_ERR,
             "API Request for DLS CREATE failed. '%s' not created.", agent->agent_tech_id );
       Json_unref ( api_result );
       return(FALSE);
     }
    Json_unref ( api_result );
    return(TRUE);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
