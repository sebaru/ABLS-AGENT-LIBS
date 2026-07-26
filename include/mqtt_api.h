/******************************************************************************************************************************/
/* include/mqtt_api.h    Déclaration des prototypes MQTT API — abls-agent-libs                                              */
/* Projet Abls-Habitat                               Gestion d'habitat                                       26.07.2026       */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * mqtt_api.h
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

#ifndef _ABLS_AGENT_LIBS_MQTT_API_H_
 #define _ABLS_AGENT_LIBS_MQTT_API_H_

 #include <glib.h>
 #include <json-glib/json-glib.h>

 #include "agent.h"

 extern JsonNode *Agent_get_mqtt_api_message  ( struct ABLS_AGENT *agent );
 extern void      Agent_send_mqtt_api_message ( struct ABLS_AGENT *agent, JsonNode *node, gboolean retain, gchar *topic, ... );

#endif /* _ABLS_AGENT_LIBS_MQTT_API_H_ */
/*----------------------------------------------------------------------------------------------------------------------------*/