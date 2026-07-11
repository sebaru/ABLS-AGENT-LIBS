/******************************************************************************************************************************/
/* ABLS-AGENT-LIBS/signal.c    Gestion des signaux de l'agent                                                                */
/* Projet Abls-Habitat                               Gestion d'habitat                                                        */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * signal.c
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

 #define _GNU_SOURCE

 #include <string.h>
 #include <signal.h>
 #include <sys/prctl.h>

 #include "abls-agent-libs.h"

 static struct ABLS_AGENT *local_agent = NULL;

/******************************************************************************************************************************/
/* Traitement_signaux: Gestion des signaux de controle du systeme                                                             */
/* Entree: numero du signal a gerer                                                                                           */
/******************************************************************************************************************************/
 static void Traitement_signaux( int num )
  { char chaine[50] = "?";
    prctl(PR_GET_NAME, chaine, 0, 0, 0 );

    if (!local_agent) return;
    Info( __func__, "signal", local_agent->agent_tech_id, LOG_NOTICE, "Signal '%s' received", strsignal(num) );

    switch (num)
     { case SIGQUIT:
       case SIGINT:  local_agent->Agent_run = FALSE;
                     break;
       case SIGTERM: local_agent->Agent_run = FALSE;
                     break;
       case SIGABRT: break;
       case SIGPIPE: break;
       default:      break;
     }
  }
/******************************************************************************************************************************/
/* Agent_setup_signals: Active la gestion des signaux pour l'agent courant                                                    */
/* Entree: structure agent                                                                                                    */
/* Sortie: neant                                                                                                              */
/******************************************************************************************************************************/
 void Agent_enable_signals ( struct ABLS_AGENT *agent )
  { struct sigaction sig;
    if (!agent) return;

    local_agent = agent;

    memset ( &sig, 0, sizeof(sig) );
    sig.sa_handler = Traitement_signaux;
    sig.sa_flags = SA_RESTART;
    sigemptyset ( &sig.sa_mask );

    sigaction ( SIGQUIT, &sig, NULL );
    sigaction ( SIGINT,  &sig, NULL );
    sigaction ( SIGTERM, &sig, NULL );
    sigaction ( SIGABRT, &sig, NULL );
    sigaction ( SIGPIPE, &sig, NULL );
    Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_INFO, "Signal handlers installed" );
  }
/******************************************************************************************************************************/
/* Agent_disable_signals: Desactive la gestion des signaux pour l'agent courant                                               */
/* Entree: structure agent                                                                                                    */
/* Sortie: neant                                                                                                              */
/******************************************************************************************************************************/
 void Agent_disable_signals ( void )
  { struct sigaction sig;
    if (!local_agent) return;

    memset ( &sig, 0, sizeof(sig) );
    sig.sa_handler = SIG_DFL;
    sig.sa_flags = SA_RESTART;
    sigemptyset ( &sig.sa_mask );

    sigaction ( SIGQUIT, &sig, NULL );
    sigaction ( SIGINT,  &sig, NULL );
    sigaction ( SIGTERM, &sig, NULL );
    sigaction ( SIGABRT, &sig, NULL );
    sigaction ( SIGPIPE, &sig, NULL );

    Info( __func__, local_agent->agent_classe, local_agent->agent_tech_id, LOG_INFO, "Signal handlers disabled" );
    local_agent = NULL;
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
