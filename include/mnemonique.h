/******************************************************************************************************************************/
/* include/mnemonique.h Déclaration des prototypes Mnemonique — abls-agent-libs                                               */
/* Projet Abls-Habitat                               Gestion d'habitat                                       03.07.2026       */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * mnemonique.h
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

#ifndef _ABLS_AGENT_LIBS_MNEMONIQUE_H_
 #define _ABLS_AGENT_LIBS_MNEMONIQUE_H_

 extern JsonNode *Mnemo_create_AI                 ( struct ABLS_AGENT *agent, gchar *agent_acronyme, gchar *libelle, gchar *unite, gint archivage );
 extern JsonNode *Mnemo_create_DI                 ( struct ABLS_AGENT *agent, gchar *agent_acronyme, gchar *libelle );
 extern JsonNode *Mnemo_create_CI                 ( struct ABLS_AGENT *agent, gchar *agent_acronyme, gchar *libelle, gchar *unite, gint archivage );
 extern JsonNode *Mnemo_create_DO                 ( struct ABLS_AGENT *agent, gchar *agent_acronyme, gchar *libelle, gboolean mono );
 extern JsonNode *Mnemo_create_AO                 ( struct ABLS_AGENT *agent, gchar *agent_acronyme, gchar *libelle, gchar *unite, gint archivage );
 extern JsonNode *Mnemo_create_HORLOGE            ( struct ABLS_AGENT *agent, gchar *agent_acronyme, gchar *libelle );
 extern void      Mnemo_create_HORLOGE_tick       ( struct ABLS_AGENT *agent, JsonNode *bit, gint heure, gint minute );
 extern void      Mnemo_delete_thread_HORLOGE_tick( struct ABLS_AGENT *agent, JsonNode *bit );
 extern JsonNode *Mnemo_create_WATCHDOG           ( struct ABLS_AGENT *agent, gchar *agent_acronyme, gchar *libelle );

#endif /* _ABLS_AGENT_LIBS_MNEMONIQUE_H_ */
/*----------------------------------------------------------------------------------------------------------------------------*/
