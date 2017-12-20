#ifndef BATTLE_EVENTS_H_
#define BATTLE_EVENTS_H_

enum EventStatesWildBattle {
    EventFlee = 0, // 0
    EventSwitch, // 1
    EventBeforeMove, // 2
    EventMoveTargetConfig, // 3
    EventPreMoveHit, // 4
    EventMoveTryHit, // 5
    EventMoveDamage, // 6
    EventMoveHeal, // 7
    EventMoveEffect, // 8
    EventMoveRecoil, // 9
    EventMoveDrain, // 10
    EventMoveBoosts, // 11
    EventMoveAilments, // 12
    EventMoveMultiHitResolve, // 13
    EventMoveAfterMove, // 14
    EventDoFaints, // 15
    EventMoveFailed, // 16
    EventPeekMessage, // 17
    EventDamageAnim, // 18
    EventHealAnim, // 19
    EventFaint, // 20
    EventEndAction, // 21
    EventInactive, // 22
    EventResidualEffects, // 23
    EventWildBattleOver, // 24
    EventEndBattle, // 25
};

extern void event_run_flee(struct action* a);
extern void event_switch(struct action* a);
extern void event_before_move(struct action* a);
extern void event_pre_move_hit(struct action* a);
extern void event_config_move_hitlist(struct action* a);
extern void event_pre_move_hit(struct action* current_action);
extern void event_move_tryhit_external(struct action* current_action);
extern void event_move_damage(struct action* current_action);
extern void event_move_heal(struct action* current_action);
extern void event_move_effect(struct action* current_action);
extern void event_move_recoil(struct action* current_action);
extern void event_move_drain(struct action* current_action);
extern void event_move_boosts(struct action* current_action);
extern void event_move_ailments(struct action* current_action);
extern void event_multi_hit(struct action* current_action);
extern void event_after_move(struct action* current_action);
extern void event_fainted(struct action* current_action);

extern void event_move_failed(struct action* current_action);
extern void event_peek_message(struct action* current_action);
extern void init_damage(struct action* current_action);
extern void init_heal(struct action* current_action);
extern void event_faint(struct action* current_action);
extern void event_set_inactive(struct action* current_action);
extern void event_residual_effects(struct action* current_action);
extern void wild_battle_status_update(struct action* current_action);
extern void end_battle(struct action* a);

#endif /* BATTLE_EVENTS_H_ */
