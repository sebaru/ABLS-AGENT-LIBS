/******************************************************************************************************************************/
/* ABLS-AGENT-LIBS/http.c        Fonctions communes de gestion des requetes HTTP                                             */
/* Projet Abls-Habitat                   Gestion d'habitat                                                30.12.2020 22:03:58 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * http.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2026 - Sébastien LEFÈVRE
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

 #include <glib.h>
 #include <stdio.h>
 #include <curl/curl.h>

 #include "abls-agent-libs.h"

 struct HTTP_BUFFER
  { gchar *body;
    size_t size;
  };

/******************************************************************************************************************************/
/* Http_Init: Initialise la librairie HTTP                                                                                    */
/* Entrée: l'agent en cours d'execution                                                                                       */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Http_Init ( struct ABLS_AGENT *agent )
  { curl_global_init(CURL_GLOBAL_DEFAULT);
    g_mkdir ( "http_cache", 0755 );
    Info( __func__, "http", agent->agent_tech_id, LOG_DEBUG, "lib cURL initialized" );
  }
/******************************************************************************************************************************/
/* Http_End: Désactive la librairie HTTP                                                                                      */
/* Entrée: l'agent en cours d'execution                                                                                       */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Http_End ( struct ABLS_AGENT *agent )
  { curl_global_cleanup();
    Info( __func__, "http", agent->agent_tech_id, LOG_DEBUG, "lib cURL un-initialized" );
  }
/******************************************************************************************************************************/
/* Http_Write_CB: Fonction d'écriture du callback                                                                             */
/* Entrée: l'agent en cours d'execution, le contenu, la taille, le nombre de bytes, de user data                              */
/* Sortie: la nouvelle taille                                                                                                 */
/******************************************************************************************************************************/
 static size_t Http_Write_CB ( struct ABLS_AGENT *agent, void *contents, size_t size, size_t nmemb, void *userp )
  { struct HTTP_BUFFER *buffer = userp;
    size_t chunksize = size * nmemb;

    char *ptr = g_try_realloc( buffer->body, buffer->size + chunksize + 1 );
    if(!ptr)
     { Info( __func__, "http", agent->agent_tech_id, LOG_ERR, "Realloc failed" ); return(0); }

    buffer->body = ptr;
    memcpy( buffer->body + buffer->size, contents, chunksize );
    buffer->size += chunksize;
    buffer->body[buffer->size] = 0;
    return(chunksize);
  }
/******************************************************************************************************************************/
/* Http_Add_signature: Ajoute les headers et la signature a une requete CURL                                                  */
/* Entrée: L'agent, le CURL et le payload prévu                                                                               */
/* Sortie: aucune, les headers et signatures sont mis à jour dans le CURL                                                     */
/******************************************************************************************************************************/
 static void Http_Add_signature ( struct ABLS_AGENT *agent, CURL *curl, gchar *payload )
  { struct curl_slist *all_headers = NULL;                                                        /* Gestion des headers HTTP */
    gchar timestamp[20];                                                                 /* On récupère la date de la requete */
    g_snprintf( timestamp, sizeof(timestamp), "%ld", time(NULL) );

/*------------------------------------------------ Préparation des headers ---------------------------------------------------*/
    all_headers = curl_slist_append( all_headers, "Content-Type: application/json" );
    gchar chaine[256];
    g_snprintf( chaine, sizeof(chaine), "Origin: %s", "abls-habitat.fr" );
    all_headers = curl_slist_append( all_headers, chaine );

    g_snprintf( chaine, sizeof(chaine), "X-ABLS-DOMAIN: %s", agent->domain_uuid );
    all_headers = curl_slist_append( all_headers, chaine );

    g_snprintf( chaine, sizeof(chaine), "X-ABLS-AGENT-TECH-ID: %s", agent->agent_tech_id );
    all_headers = curl_slist_append( all_headers, chaine );

    g_snprintf( chaine, sizeof(chaine), "X-ABLS-TIMESTAMP: %s", timestamp );
    all_headers = curl_slist_append( all_headers, chaine );

/*---------------------------------------------- Calcul de la signature ------------------------------------------------------*/
    unsigned char hash_bin[EVP_MAX_MD_SIZE];
    gint md_len;
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();                                                                   /* Calcul du SHA1 */
    EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(mdctx, agent->domain_uuid,   strlen(agent->domain_uuid));
    EVP_DigestUpdate(mdctx, agent->agent_tech_id, strlen(agent->agent_tech_id));
    EVP_DigestUpdate(mdctx, agent->domain_secret, strlen(agent->domain_secret));
    if (payload) EVP_DigestUpdate(mdctx, payload, strlen(payload));
    EVP_DigestUpdate(mdctx, timestamp,            strlen(timestamp));
    EVP_DigestFinal_ex(mdctx, hash_bin, &md_len);
    EVP_MD_CTX_free(mdctx);
    gchar signature[64];
    EVP_EncodeBlock( signature, hash_bin, 32 );                                 /* Encodage et signature 256 bits -> 32 bytes */

    g_snprintf( chaine, sizeof(chaine), "X-ABLS-SIGNATURE: %s", signature );
    all_headers = curl_slist_append( all_headers, chaine );

/*------------------------------------------- Fin des hearders ---------------------------------------------------------------*/
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, all_headers);
  }
/******************************************************************************************************************************/
/* Http_Query: Réalise une requete HTTP                                                                                       */
/* Entrée : l'agent et la structure CURL                                                                                      */
/* Sortie : le noeud JSON de la réponse                                                                                       */
/******************************************************************************************************************************/
 static JsonNode *Http_Query ( struct ABLS_AGENT *agent, CURL *curl )
  { JsonNode *ReponseNode = NULL;

    struct HTTP_BUFFER *buffer = g_try_malloc0( sizeof(struct HTTP_BUFFER) );     /* Buffer temporaire de récup de la reponse */
    if (!buffer) { Info( __func__, "http", agent->agent_tech_id, LOG_ERR, "Request to %s: Malloc buffer failed", url ); goto end; }
    curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, Http_Write_CB );
    curl_easy_setopt( curl, CURLOPT_WRITEDATA, buffer );

    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);                               /* Recupération du code retour */
    Info( __func__, "http", agent->agent_tech_id, LOG_DEBUG, "Request to %s: HttpCode = %d", url, http_code );

    if( res != CURLE_OK )
     { Info( __func__, "http", agent->agent_tech_id, LOG_ERR, "Request to %s: Curl_easy_perform failed: %s", url, curl_easy_strerror(res) ); }
    else if (http_code == 200)
     { if (buffer->body)
        { ReponseNode = Json_get_from_string ( buffer->body );
          if (!ReponseNode) { Info( __func__, "http", agent->agent_tech_id, LOG_ERR, "Request to %s: Response is not json", url ); }
        }
       else Info( __func__, "http", agent->agent_tech_id, LOG_ERR, "No body received" );
     }
    else Info( __func__, "http", agent->agent_tech_id, LOG_ERR, "Request to %s: HttpCode = %d", url, http_code );
end:
    if (buffer)                                                                                                 /* Si reponse */
     { if(buffer->body) g_free(buffer->body);                                                           /* On free la réponse */
       g_free(buffer);
     }

    if (!ReponseNode) ReponseNode = Json_create ();                            /* Si pas de body en response, on en créé un */
    if (ReponseNode) { Json_add_int ( ReponseNode, "http_code", http_code ); }
    else Info( __func__, "http", agent->agent_tech_id, LOG_ERR, "Memory Error" );
    return(ReponseNode);
  }
/******************************************************************************************************************************/
/* Http_Post_to_global_API: Réalise une requete POST vers l'API globale                                                       */
/* Entrée: l'agent, l'url, le payload                                                                                         */
/* Sortie: la reponse json                                                                                                    */
/******************************************************************************************************************************/
 JsonNode *Http_Post_to_global_API ( struct ABLS_AGENT *agent, gchar *uri, JsonNode *json_payload )
  { struct curl_slist *all_headers = NULL;                                                        /* Gestion des headers HTTP */
    JsonNode *ReponseNode = NULL;
    gchar *payload = NULL;
    gint http_code = 0;                                                                                     /* Code de retour */

    gchar url[256];
    if (uri) g_snprintf( url, sizeof(url), "https://%s%s", agent->api_url, uri );
        else g_snprintf( url, sizeof(url), "https://%s", agent->api_url );

    Info( __func__, "http", agent->agent_tech_id, LOG_DEBUG, "Request to %s is starting", url );
/*------------------------------------------------ Init du cURL --------------------------------------------------------------*/
    CURL *curl = curl_easy_init();
    if(!curl) { Info( __func__, "http", agent->agent_tech_id, LOG_ERR, "Request to %s: Curl_easy_init failed", url ); return(NULL); }

/*------------------------------------------------ Préparation du payload ----------------------------------------------------*/
    if (json_payload)
     { payload = Json_to_string ( json_payload );
       if (!payload) { Info( __func__, "http", agent->agent_tech_id, LOG_ERR, "Request to %s: Json to string failed", url ); goto end; }
     }

/*------------------------------------------------ Préparation du cURL -------------------------------------------------------*/
    curl_easy_setopt( curl, CURLOPT_URL, url );
    if (payload)
     { curl_easy_setopt( curl, CURLOPT_POST, 1L );
       curl_easy_setopt( curl, CURLOPT_POSTFIELDS, payload );
     }

/*------------------------------------------------ Execution du cURL ---------------------------------------------------------*/
    Http_Add_signature ( agent, curl, payload );                                        /* Ajoute les headers et la signature */
    JsonNode *ReponseNode = Http_Query ( agent, curl );                                                 /* Réalise la requete */

end:
    if (payload) g_free(payload);                                                                /* free the "string" payload */
    curl_easy_cleanup(curl);
    return(ReponseNode);
  }
/******************************************************************************************************************************/
/* Http_Query_to_cache: transforme une query en nom de fichier cache                                                          */
/* Entrée: la query                                                                                                           */
/* Sortie: le cache filename                                                                                                  */
/******************************************************************************************************************************/
 static gchar *Http_Query_to_cache ( struct ABLS_AGENT *agent, gchar *query )
  { gint taille_nom_fichier = 256;
    gchar *nom_fichier = g_try_malloc0(taille_nom_fichier);
    if (!nom_fichier)
     { Info( __func__, "http", agent->agent_tech_id, LOG_ERR, "Memory error for Caching %s", query );
       return(NULL);
     }
    g_snprintf ( nom_fichier, taille_nom_fichier, "http_cache/%s", query );
    g_strcanon ( nom_fichier+11, "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWYYZ", '_' );
    return(nom_fichier);
  }
/******************************************************************************************************************************/
/* Http_Get_from_global_API: Récupère la partie payload auprès de l'API                                                       */
/* Entrée: le messages                                                                                                        */
/* Sortie: le Json                                                                                                            */
/******************************************************************************************************************************/
 JsonNode *Http_Get_from_global_API ( struct ABLS_AGENT *agent, gchar *URI, gchar *format, ... )
  { gchar url[512];
    va_list ap;

    if (format)
     { gchar parametres[128];
       va_start( ap, format );
       g_vsnprintf ( parametres, sizeof(parametres), format, ap );
       va_end ( ap );
       g_snprintf( url, sizeof(url), "https://%s/%s?%s", Json_get_string ( Config.config, "api_url"), URI, parametres );
     }
    else g_snprintf( url, sizeof(url), "https://%s/%s", Json_get_string ( Config.config, "api_url"), URI );

/*------------------------------------------------ Init du cURL --------------------------------------------------------------*/
    CURL *curl = curl_easy_init();
    if(!curl) { Info( __func__, "http", agent->agent_tech_id, LOG_ERR, "Request to %s: Curl_easy_init failed", url ); return(NULL); }

/*------------------------------------------------ Préparation du cURL -------------------------------------------------------*/
    curl_easy_setopt( curl, CURLOPT_URL, url );

/*------------------------------------------------ Execution du cURL ---------------------------------------------------------*/
    Http_Add_signature ( agent, curl, payload );                                        /* Ajoute les headers et la signature */
    JsonNode *ReponseNode = Http_Query ( agent, curl );                                                 /* Réalise la requete */
    if (!ReponseNode) { Info( __func__, "http", agent->agent_tech_id, LOG_ERR, "Error with Http_Get %s", url ); goto end; }
    gint http_code = Json_get_int ( ReponseNode, "http_code" );
    Info( __func__, "http", agent->agent_tech_id, LOG_DEBUG, "%s Status %d for '%s'", URI, http_code, url );

    if (http_code!=200)
     { Info( __func__, "http", agent->agent_tech_id, LOG_ERR, "%s Error %d for '%s'", URI, http_code, url );
       Json_unref ( ReponseNode );                                        /* plus besoin de la response puisque code d'erreur */
       gchar *nom_fichier = Http_Query_to_cache ( agent, url );
       if (nom_fichier)
        { ReponseNode = Json_read_from_file ( nom_fichier );                                      /* Trying to get from cache */
          g_free(nom_fichier);
          if (ReponseNode) Info( __func__, "http", agent->agent_tech_id, LOG_INFO, "Using cache for %s OK", url );
                      else Info( __func__, "http", agent->agent_tech_id, LOG_ERR,  "Using cache for %s failed", url );
        } else Info( __func__, "http", agent->agent_tech_id, LOG_ERR, "Cache error for %s", url );
     }
    else
     { if (Json_has_member ( ReponseNode, "api_cache" ) && Json_get_bool ( ReponseNode, "api_cache" ) )
        { gchar *nom_fichier = Http_Query_to_cache ( agent, url );
          if (nom_fichier) { Json_write_to_file ( nom_fichier, ReponseNode ); g_free(nom_fichier); }
        }
     }
end:
    curl_easy_cleanup(curl);
    return(ReponseNode);
 }
/*----------------------------------------------------------------------------------------------------------------------------*/
