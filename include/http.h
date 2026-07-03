/******************************************************************************************************************************/
/* include/http.h      Déclaration des prototypes HTTP — abls-agent-libs                                                      */
/* Projet Abls-Habitat                               Gestion d'habitat                                       03.07.2026       */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * http.h
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

#ifndef _ABLS_AGENT_LIBS_HTTP_H_
 #define _ABLS_AGENT_LIBS_HTTP_H_

 #include <glib.h>
 #include <json-glib/json-glib.h>

 struct ABLS_AGENT;

 extern void      Http_Init               ( struct ABLS_AGENT *agent );
 extern void      Http_End                ( struct ABLS_AGENT *agent );
 extern JsonNode *Http_Post_to_global_API ( struct ABLS_AGENT *agent, gchar *uri, JsonNode *json_payload );
 extern JsonNode *Http_Get_from_global_API( struct ABLS_AGENT *agent, gchar *URI, gchar *format, ... );

#endif /* _ABLS_AGENT_LIBS_HTTP_H_ */
/*----------------------------------------------------------------------------------------------------------------------------*/
