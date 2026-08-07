/******************************************************************************************************************************/
/* src/thread.c       Gestion de la queue de commandes thread                                                                 */
/* Projet Abls-Habitat                               Gestion d'habitat                                                        */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * thread.c
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

 #include <stdarg.h>

 #include "abls-agent-libs.h"

/******************************************************************************************************************************/
/* Thread_shell_queue_exec: Consomme la queue de textes de l'agent                                                            */
/* Entree: pointeur agent                                                                                                     */
/* Sortie: NULL                                                                                                               */
/******************************************************************************************************************************/
 static gpointer Thread_shell_queue_exec ( gpointer user_data )
  { struct ABLS_AGENT *agent = user_data;

    if (!agent) return NULL;

    while (agent->Agent_run == AGENT_IS_RUNNING)
     { gchar *text = g_async_queue_try_pop ( agent->Thread_shell_queue );
       if (text) { Exec (text); g_free ( text ); }
       else break;
     }

    g_rw_lock_writer_lock ( &agent->Thread_shell_queue_lock );
    agent->Thread_shell = NULL;
    g_rw_lock_writer_unlock ( &agent->Thread_shell_queue_lock );
    return(NULL);
  }
/******************************************************************************************************************************/
/* Thread_shell_queue: Enfile une chaine de caracteres et demarre le worker a la demande                                      */
/* Entree: structure agent, chaine formattee variadique                                                                       */
/* Sortie: aucune                                                                                                             */
/******************************************************************************************************************************/
 void Thread_shell_queue ( struct ABLS_AGENT *agent, gchar *name, const gchar *format, ... )
  { gchar *queued;
    va_list ap;

    if (!agent || !format) return;
    if (agent->Agent_run != AGENT_IS_RUNNING || !agent->Thread_shell_queue) return;

    va_start ( ap, format );
    queued = g_strdup_vprintf ( format, ap );
    va_end ( ap );

    if (!queued)
     { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_ERR, "Unable to duplicate queued text" );
       return;
     }

    g_async_queue_push ( agent->Thread_shell_queue, queued );

    g_rw_lock_reader_lock ( &agent->Thread_shell_queue_lock );
    gboolean worker_running = (agent->Thread_shell != NULL);
    g_rw_lock_reader_unlock ( &agent->Thread_shell_queue_lock );
    if (worker_running) return;

    g_rw_lock_writer_lock ( &agent->Thread_shell_queue_lock );
    agent->Thread_shell = g_thread_new ( name, Thread_shell_queue_exec, agent );
    g_rw_lock_writer_unlock ( &agent->Thread_shell_queue_lock );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/