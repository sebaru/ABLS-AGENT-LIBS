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
 #include "agent_private.h"

/* Un handler s'exécute n'importe quand, y compris au milieu d'un malloc ou d'un Info du thread interrompu. Il ne fait donc   */
/* que des opérations atomiques sur les variables statiques ci-dessous (jamais sur la structure agent, qui peut être libérée).*/
/* Le reste (logs, dumps de config) est traité plus tard, hors handler, par Agent_process_signals.                            */
 #ifndef G_ATOMIC_LOCK_FREE
  #error "Les opérations atomiques de GLib doivent être lock-free pour être utilisées dans un handler de signal"
 #endif

 static struct ABLS_AGENT *local_agent = NULL;                                  /* Jamais lu par le handler : logs uniquement */
 static const gint Signal_list[] = { SIGQUIT, SIGINT, SIGTERM, SIGPIPE, SIGUSR1 };      /* Signaux comptés puis traités après */
 static gint  Signal_count[NSIG] = { 0 };                                    /* Réceptions non traitées, par numéro de signal */
 static gint  Signal_stop_requested = FALSE;                                              /* TRUE dès qu'un arret est demandé */
 static guint Signal_top = 0;                                                                           /* dixième de seconde */

/******************************************************************************************************************************/
/* Traitement_signaux: Gestion des signaux de controle du systeme (atomiques uniquement, sûr dans un handler)                 */
/* Entree: numero du signal a gerer                                                                                           */
/******************************************************************************************************************************/
 static void Traitement_signaux( int num )
  { if (num == SIGALRM) { g_atomic_int_inc ( &Signal_top ); return; }                                     /* Gestion du timer */

    if (num == SIGQUIT || num == SIGINT || num == SIGTERM) g_atomic_int_set ( &Signal_stop_requested, TRUE );
    if (num > 0 && num < NSIG) g_atomic_int_inc ( &Signal_count[num] );
  }
/******************************************************************************************************************************/
/* Agent_process_signals: Traite, hors handler, les signaux reçus depuis le dernier appel (logs, dump de config)              */
/* Entree: structure agent                                                                                                    */
/* Sortie: neant                                                                                                              */
/******************************************************************************************************************************/
 void Agent_process_signals ( struct ABLS_AGENT *agent )
  { if (!agent) return;
    for (guint cpt = 0; cpt < G_N_ELEMENTS(Signal_list); cpt++)         /* Process uniquement les signaux de la liste validée */
     { gint num = Signal_list[cpt];                                               /* Récupère le numéro du signal a controler */
       gint nbr = g_atomic_int_get ( &Signal_count[num] );                      /* Chaque réception n'est traitée qu'une fois */
       if (nbr > 0)
        { g_atomic_int_set ( &Signal_count[num], 0 );                                            /* RAZ du compteur du signal */
          Info( __func__, "signal", agent->agent_tech_id, LOG_NOTICE, "Signal '%s' received (%d times)", strsignal(num), nbr );
          switch ( num )
           { case SIGUSR1:
              { Json_to_log ( __func__, "dumping_local_config", agent->agent_tech_id, LOG_ALERT, agent->local_config );
                Json_to_log ( __func__, "dumping_api_config",   agent->agent_tech_id, LOG_ALERT, agent->api_config   );
                break;
              }
             case SIGQUIT:
             case SIGINT:
             case SIGTERM:
              { agent->Agent_run = AGENT_NEED_TO_STOP;
                break;
              }
           }
        }
     }
  }
/******************************************************************************************************************************/
/* Agent_signals_get_top: Retourne le nombre de dixièmes de seconde écoulés depuis l'activation des signaux                  */
/* Entree: neant                                                                                                              */
/* Sortie: le compteur                                                                                                        */
/******************************************************************************************************************************/
 guint Agent_signals_get_top ( void )
  { return ( (guint)g_atomic_int_get ( &Signal_top ) ); }
/******************************************************************************************************************************/
/* Agent_signals_stop_requested: Indique si un signal d'arret (QUIT, INT ou TERM) a été reçu                                  */
/* Entree: neant                                                                                                              */
/* Sortie: TRUE si l'arret est demandé                                                                                        */
/******************************************************************************************************************************/
 gboolean Agent_signals_stop_requested ( void )
  { return ( g_atomic_int_get ( &Signal_stop_requested ) != FALSE ); }
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
    for (guint cpt = 0; cpt < G_N_ELEMENTS(Signal_list); cpt++)                            /* Pour tous les signaux a catcher */
     { sigaction ( Signal_list[cpt], &sig, NULL ); }
    sigaction ( SIGALRM, &sig, NULL );                                               /* Timer : hors Signal_list (non compté) */

    Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_INFO, "Signal handlers installed" );

/****************************************************** Démarrage du timer ****************************************************/
    struct itimerval timer;
    timer.it_value.tv_sec  = timer.it_interval.tv_sec  = 0;                                     /* Tous les 100 millisecondes */
    timer.it_value.tv_usec = timer.it_interval.tv_usec = 100000;                                    /* = 10 fois par secondes */
    setitimer( ITIMER_REAL, &timer, NULL );                                                                /* Active le timer */
    Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_INFO,
          "Timer with interval %dms started", (gint)timer.it_value.tv_usec/1000 );
  }
/******************************************************************************************************************************/
/* Agent_disable_signals: Desactive la gestion des signaux pour l'agent courant                                               */
/* Entree: structure agent                                                                                                    */
/* Sortie: neant                                                                                                              */
/******************************************************************************************************************************/
 void Agent_disable_signals ( void )
  { struct sigaction sig;
    if (!local_agent) return;

/****************************************************** Arret du timer ********************************************************/
    struct itimerval timer;
    memset ( &timer, 0, sizeof(timer) );                           /* Coupé en premier : plus aucun SIGALRM ne peut arriver */
    setitimer( ITIMER_REAL, &timer, NULL );

    memset ( &sig, 0, sizeof(sig) );
    sig.sa_handler = SIG_DFL;
    sig.sa_flags = SA_RESTART;
    sigemptyset ( &sig.sa_mask );

    sigaction ( SIGQUIT, &sig, NULL );                                                 /* Un second arret demandé tue l'agent */
    sigaction ( SIGINT,  &sig, NULL );
    sigaction ( SIGTERM, &sig, NULL );
/* SIGALRM, SIGPIPE et SIGUSR1 gardent leur handler inoffensif : ils ne doivent jamais tuer l'agent pendant son arret.        */

    Info( __func__, local_agent->agent_classe, local_agent->agent_tech_id, LOG_INFO, "Signal handlers disabled" );
    Info( __func__, local_agent->agent_classe, local_agent->agent_tech_id, LOG_INFO, "Timer stopped" );

    local_agent = NULL;
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
