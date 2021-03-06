/*
 * Copyright (c) 2010-2011 Broadcom Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef VCHIQ_ARM_H
#define VCHIQ_ARM_H

#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <linux/atomic.h>
#include "vchiq_core.h"


enum vc_suspend_status {
	VC_SUSPEND_FORCE_CANCELED = -3, /* Force suspend canceled, too busy */
	VC_SUSPEND_REJECTED = -2,  /* Videocore rejected suspend request */
	VC_SUSPEND_FAILED = -1,    /* Videocore suspend failed */
	VC_SUSPEND_IDLE = 0,       /* VC active, no suspend actions */
	VC_SUSPEND_REQUESTED,      /* User has requested suspend */
	VC_SUSPEND_IN_PROGRESS,    /* Slot handler has recvd suspend request */
	VC_SUSPEND_SUSPENDED       /* Videocore suspend succeeded */
};

enum vc_resume_status {
	VC_RESUME_FAILED = -1, /* Videocore resume failed */
	VC_RESUME_IDLE = 0,    /* VC suspended, no resume actions */
	VC_RESUME_REQUESTED,   /* User has requested resume */
	VC_RESUME_IN_PROGRESS, /* Slot handler has received resume request */
	VC_RESUME_RESUMED      /* Videocore resumed successfully (active) */
};


enum USE_TYPE_E {
	USE_TYPE_SERVICE,
	USE_TYPE_SERVICE_NO_RESUME,
	USE_TYPE_VCHIQ
};



typedef struct vchiq_arm_state_struct {
	/* Keepalive-related data */
	struct task_struct *ka_thread;
	struct completion ka_evt;
	atomic_t ka_use_count;
	atomic_t ka_use_ack_count;
	atomic_t ka_release_count;

	struct completion vc_suspend_complete;
	struct completion vc_resume_complete;

	rwlock_t susp_res_lock;
	enum vc_suspend_status vc_suspend_state;
	enum vc_resume_status vc_resume_state;

	unsigned int wake_address;

	struct timer_list suspend_timer;
	int suspend_timer_timeout;
	int suspend_timer_running;

	/* Global use count for videocore.
	** This is equal to the sum of the use counts for all services.  When
	** this hits zero the videocore suspend procedure will be initiated.
	*/
	int videocore_use_count;

	/* Use count to track requests from videocore peer.
	** This use count is not associated with a service, so needs to be
	** tracked separately with the state.
	*/
	int peer_use_count;

	/* Flag to indicate whether resume is blocked.  This happens when the
	** ARM is suspending
	*/
	struct completion resume_blocker;
	int resume_blocked;
	struct completion blocked_blocker;
	int blocked_count;

	int autosuspend_override;

	/* Flag to indicate that the first vchiq connect has made it through.
	** This means that both sides should be fully ready, and we should
	** be able to suspend after this point.
	*/
	int first_connect;
	
	unsigned long long suspend_start_time;
	unsigned long long sleep_start_time;
	unsigned long long resume_start_time;
	unsigned long long last_wake_time;

} VCHIQ_ARM_STATE_T;

extern int vchiq_arm_log_level;
extern int vchiq_susp_log_level;

extern int __init
vchiq_platform_init(VCHIQ_STATE_T *state);

extern void __exit
vchiq_platform_exit(VCHIQ_STATE_T *state);

extern VCHIQ_STATE_T *
vchiq_get_state(void);

extern VCHIQ_STATUS_T
vchiq_arm_vcsuspend(VCHIQ_STATE_T *state);

extern VCHIQ_STATUS_T
vchiq_arm_force_suspend(VCHIQ_STATE_T *state);

extern int
vchiq_arm_allow_resume(VCHIQ_STATE_T *state);

extern VCHIQ_STATUS_T
vchiq_arm_vcresume(VCHIQ_STATE_T *state);

extern VCHIQ_STATUS_T
vchiq_arm_init_state(VCHIQ_STATE_T *state, VCHIQ_ARM_STATE_T *arm_state);

extern int
vchiq_check_resume(VCHIQ_STATE_T *state);

extern void
vchiq_check_suspend(VCHIQ_STATE_T *state);

extern VCHIQ_STATUS_T
vchiq_use_service(VCHIQ_SERVICE_HANDLE_T handle);

extern VCHIQ_STATUS_T
vchiq_release_service(VCHIQ_SERVICE_HANDLE_T handle);

extern VCHIQ_STATUS_T
vchiq_check_service(VCHIQ_SERVICE_T *service);

extern VCHIQ_STATUS_T
vchiq_platform_suspend(VCHIQ_STATE_T *state);

extern int
vchiq_platform_videocore_wanted(VCHIQ_STATE_T *state);

extern int
vchiq_platform_use_suspend_timer(void);

extern void
vchiq_dump_platform_use_state(VCHIQ_STATE_T *state);

extern void
vchiq_dump_service_use_state(VCHIQ_STATE_T *state);

extern VCHIQ_ARM_STATE_T*
vchiq_platform_get_arm_state(VCHIQ_STATE_T *state);

extern int
vchiq_videocore_wanted(VCHIQ_STATE_T *state);

extern VCHIQ_STATUS_T
vchiq_use_internal(VCHIQ_STATE_T *state, VCHIQ_SERVICE_T *service,
		enum USE_TYPE_E use_type);
extern VCHIQ_STATUS_T
vchiq_release_internal(VCHIQ_STATE_T *state, VCHIQ_SERVICE_T *service);

void
set_suspend_state(VCHIQ_ARM_STATE_T *arm_state,
	enum vc_suspend_status new_state);

void
set_resume_state(VCHIQ_ARM_STATE_T *arm_state,
	enum vc_resume_status new_state);

void
start_suspend_timer(VCHIQ_ARM_STATE_T *arm_state);

extern int vchiq_proc_init(void);
extern void vchiq_proc_deinit(void);
extern struct proc_dir_entry *vchiq_proc_top(void);
extern struct proc_dir_entry *vchiq_clients_top(void);


#endif /* VCHIQ_ARM_H */
