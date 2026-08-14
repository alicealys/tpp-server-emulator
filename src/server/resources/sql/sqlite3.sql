-- query:mgstpp.players.create
create table if not exists `players`
(
	id									integer	primary key autoincrement,
	account_id							bigint unsigned		 		 unique,
	session_id							char(32)		default null unique,
	login_password						char(32)		default null,
	last_update							datetime		default null,
	crypto_key							char(32)		default null,
	smart_device_id						char(128)		default null,
	currency							varchar(32)		default null,
	ex_ip								varchar(15)		default null,
	in_ip								varchar(15)		default null,
	ex_port								int unsigned	default 0,
	in_port								int unsigned	default 0,
	nat									int unsigned	default 0,
	creation_time						datetime        not null,
	security_challenge					boolean			not null default false,
	current_lock						bigint unsigned	not null		default 0,
	current_sneak_mode					int unsigned	not null		default 0,
	current_sneak_fob					bigint unsigned not null		default 0,
	current_sneak_player				bigint unsigned not null		default 0,
	current_sneak_platform				int unsigned	not null		default 0,
	current_sneak_status				int unsigned	not null		default 0,
	current_sneak_is_sneak				tinyint			not null		default 0,
	current_sneak_security_challenge 	tinyint			not null		default 0,
	current_sneak_start 				datetime
)
-- query:mgstpp.players.set_auto_increment
update SQLITE_SEQUENCE set seq = {} WHERE name = 'players'
-- query:mgstpp.player_records.create
create table if not exists `player_records`
(
	id							integer	primary key autoincrement,
	player_id					bigint unsigned	not null,
	fob_grade					int not null	default 0,
	prev_fob_grade				int not null	default 0,
	fob_point					int not null	default 0,
	fob_rank					int not null	default 0,
	prev_fob_rank				int not null	default 0,
	insurance_end				datetime		default null,
	league_grade				int not null	default 0,
	prev_league_grade			int not null	default 0,
	league_rank					int not null	default 0,
	prev_league_rank			int not null	default 0,
	league_point				int not null	default 0,
	pf_point					int not null	default 0,
	pf_point_add				int not null	default 0,
	event_point					int not null	default 0,
	playtime					int not null	default 0,
	point						int not null	default 0,
	fob_defense_win				int not null	default 0,
	fob_defense_lose			int not null	default 0,
	fob_sneak_win				int not null	default 0,
	fob_sneak_lose				int not null	default 0,
	fob_deploy_emergency_count	int not null	default 0,
	has_fob						boolean not null	default 0,
	challenge_tasks				blob default null,
	daily_last					datetime default null,
	daily_last_ack				datetime default null,
	league_last_ack				datetime default null,
	daily_total					int unsigned not null default 0,
	shield_date					datetime not null,
	foreign key (`player_id`) references players(`id`),
	unique (`player_id`)
)
-- query:mgstpp.player_records.update_fob_ranking
with ranked_players as (
	select player_id, fob_point, 
		   (select count(distinct fob_point) + 1 
			from player_records pr2 
			where pr2.fob_point > pr1.fob_point) as new_rank
	from player_records pr1
)
update player_records
set fob_rank = (
    select ranked_player.new_rank
    from ranked_players ranked_player
    where ranked_player.player_id = player_records.player_id
)
where fob_point > 0;
-- query:mgstpp.player_records.update_league_ranking
with ranked_players as (
	select player_id, league_point, 
		   (select count(distinct league_point) + 1 
			from player_records pr2 
			where pr2.league_point > pr1.league_point) as new_rank
	from player_records pr1
)
update player_records
set league_rank = (
    select ranked_player.new_rank
    from ranked_players ranked_player
    where ranked_player.player_id = player_records.player_id
)
where league_point > 0;
-- query:mgstpp.player_records.fob_grade.check_index
select 0 has_index
-- query:mgstpp.player_records.fob_grade.create_index
create index if not exists `fob_grade_index` on `player_records`(`fob_grade`)
-- query:mgstpp.player_data.create
create table if not exists `player_data`
(
	id						integer	primary key autoincrement,
	player_id				bigint unsigned	not null,
	unit_counts				blob default null,
	unit_levels				blob default null,
	resource_arrays			blob default null,
	nuke_count				bigint unsigned default 0,
	nuke_destruct_count		bigint unsigned default 0,
	staff_count				int unsigned not null,
	staff_counts			blob default null,
	staff_bin				mediumblob default null,
	prison_bin				mediumblob default null,
	motherbase				blob default null,
	emblem					blob default null,
	local_gmp				int default 0,
	server_gmp				int default 0,
	loadout_gmp				int default 0,
	insurance_gmp			int default 0,
	injury_gmp				int default 0,
	cumulative_grade		int unsigned default 0,
	league_attack_item		int unsigned default 3,
	league_defense_item		int unsigned default 3,
	last_sync				datetime default null,
	mb_coin					int unsigned default 0,
	client_resource_version bigint unsigned default 0,
	client_staff_version 	bigint unsigned default 0,
	server_resource_version bigint unsigned default 0,
	server_staff_version 	bigint unsigned default 0,
	fob_deploy_damage_param json,
	foreign key (`player_id`) references players(`id`),
	unique (`player_id`)
)
-- query:mgstpp.items.create
create table if not exists `items`
(
	id                  integer	primary key autoincrement,
	player_id           bigint unsigned	not null,
	item_id				bigint unsigned	not null,
	create_date			datetime        default current_timestamp not null,
	foreign key (`player_id`) references players(`id`)
)
-- query:mgstpp.player_follows.create
create table if not exists `player_follows`
(
	id                  integer	primary key autoincrement,
	player_id	        bigint unsigned	not null,
	to_player_id	    bigint unsigned	not null,
	foreign key (`player_id`) references players(`id`),
	foreign key (`to_player_id`) references players(`id`),
	unique (`player_id`, `to_player_id`),
    check (`player_id` != `to_player_id`)
)
-- query:mgstpp.fobs.create
create table if not exists `fobs`
(
	id                  integer	primary key autoincrement,
	player_id	        bigint unsigned	not null,
	fob_index			bigint unsigned	not null,
	platform_count		int	unsigned	not null default 0,
	security_rank		int unsigned	not null default 0,
	area_id	            int unsigned	not null default 0,
	cluster_param		blob					 default null,
	construct_param		int unsigned	not null default 0,
	create_date			datetime        default current_timestamp not null,
	foreign key (`player_id`) references players(`id`)
)
-- query:mgstpp.fobs.set_auto_increment
update SQLITE_SEQUENCE set seq = {} WHERE name = 'fobs'
-- query:mgstpp.sneak_results.create
create table if not exists `sneak_results`
(
	id                  integer	primary key autoincrement,
	attacker_id	        bigint unsigned		not null,
	target_id	        bigint unsigned		not null,
	fob_id				bigint unsigned		not null,
	fob_index			bigint unsigned		not null,
	is_win				tinyint unsigned	not null,
	platform			int unsigned 		not null,
	event_data			blob default null,
	event_log			blob default null,
	create_date			datetime not null,
	foreign key (`attacker_id`) references players(`id`),
	foreign key (`target_id`) references players(`id`),
	foreign key (`fob_id`) references fobs(`id`)
)
-- query:mgstpp.wormholes.create
create table if not exists `wormholes`
(
	id                  integer	primary key autoincrement,
	player_id	        bigint unsigned	not null,
	to_player_id	    bigint unsigned	not null,
	retaliate_score	    int unsigned	not null	default 0,
	flag				int unsigned	not null	default 0,
	is_open				boolean			not null	default false,
	create_date			datetime		not null,
	foreign key (`player_id`) references players(`id`),
	foreign key (`to_player_id`) references players(`id`)
)
-- query:mgstpp.event_rankings.create
create table if not exists `event_rankings`
(
	id                  integer	primary key autoincrement,
	player_id	        bigint unsigned	not null,
	event_id	        int unsigned	not null,
	player_rank		    bigint unsigned	not null default 0,
	player_rank_number	bigint unsigned	not null default 0,
	value		        int				not null default 0,
	foreign key (`player_id`) references players(`id`),
	unique (`player_id`, `event_id`)
)
-- query:mgstpp.event_rankings.update_entries
with ranked as (
    select
        player_id,
        event_id,
        value,
        rank() over (partition by event_id order by value desc) as new_rank,
        row_number() over (partition by event_id order by value desc) as new_rank_number
    from event_rankings where event_id != {}
)
update event_rankings
set player_rank = (
    select ranked.new_rank
    from ranked, event_rankings
    where ranked.player_id = event_rankings.player_id
      and ranked.event_id = event_rankings.event_id
),
player_rank_number = (
    select ranked.new_rank_number
    from ranked, event_rankings
    where ranked.player_id = event_rankings.player_id
      and ranked.event_id = event_rankings.event_id
);
-- query:mgstpp.event_rankings.update_entries_league
with ranked as (
    select
        event_rankings.player_id,
        event_id,
        value,
        rank() over (partition by event_id order by player_records.league_grade desc, value desc) as new_rank,
		row_number() over (partition by event_id order by player_records.league_grade desc, value desc) as new_rank_number    
	from event_rankings join player_records on event_rankings.player_id = player_records.player_id where event_id = {}
)
update event_rankings
set player_rank = (
    select ranked.new_rank
    from ranked, event_rankings
    where ranked.player_id = event_rankings.player_id
      and ranked.event_id = event_rankings.event_id
),
player_rank_number = (
    select ranked.new_rank_number
    from ranked, event_rankings
    where ranked.player_id = event_rankings.player_id
      and ranked.event_id = event_rankings.event_id
);
-- query:mgstpp.mgo_characters.create
create table if not exists `mgo_characters`
(
	id						integer	primary key autoincrement,
	player_id				bigint unsigned	not null,
	character_index			int unsigned	not null,
	avatar					blob not null,
	loadouts				blob not null,
	loadout_count			int unsigned not null default 0,
	last_loadout			int unsigned not null default 0,
	name					char(32),
	player_class			int unsigned not null default 0,
	player_type				int unsigned  not null default 0,
	legendary				int unsigned  not null default 0,
	prestige				int unsigned  not null default 0,
	xp						int unsigned  not null default 0,
	permanent_unlock_list   int unsigned  not null default 0,
	create_date				datetime		default current_timestamp not null,
	foreign key (`player_id`) references players(`id`)
)
-- query:mgstpp.mgo_data.create
create table if not exists `mgo_data`
(
	id							integer	primary key autoincrement,
	player_id					bigint unsigned	not null,
	last_character_used			int unsigned default 0 not null,
	bgm_selected				int unsigned default 0 not null,
	gp_coin						int unsigned default 0 not null,
	gp_boost_mag				int unsigned default 0 not null,
	gp_boost_expire				datetime default null,
	rank_xp						int unsigned default 0 not null,
	xp_boost_mag				int unsigned default 0 not null,
	xp_boost_expire				datetime default null,
	reward_category				int unsigned default 0 not null,
	reward_id_a					int unsigned default 0 not null,
	reward_id_b					int unsigned default 0 not null,
	reward_id_c					int unsigned default 0 not null,
	survival_tickets			int unsigned default 0 not null,
	matches_played				int unsigned default 0 not null,
	matches_abandoned			int unsigned default 0 not null,
	matches_started				int unsigned default 0 not null,
	match_settings				tinyblob default null,
	preset_radio				tinyblob default null,
	foreign key (`player_id`) references players(`id`),
	unique (`player_id`)
)
-- query:mgstpp.mgo_stats.create
create table if not exists `mgo_stats`
(
	id						integer	primary key autoincrement,
	player_id				bigint unsigned	not null,
	stats					blob	not null,
	foreign key (`player_id`) references players(`id`),
	unique (`player_id`)
)
-- query:mgstpp.mgo_titles.create
create table if not exists `mgo_titles`
(
	id						integer	primary key autoincrement,
	player_id				bigint unsigned	not null,
	title_id				int unsigned	not null,
	flag					int unsigned	not null,
	gp					int unsigned	not null,
	date					datetime	not null,
	foreign key (`player_id`) references players(`id`),
	unique (`player_id`, `title_id`)
)
-- query:mgstpp.mgo_color_purchases.create
create table if not exists `mgo_color_purchases`
(
	id						integer	primary key autoincrement,
	player_id				bigint unsigned	not null,
	category				int unsigned	not null,
	item_id					int unsigned	not null,
	color_id				int unsigned	not null,
	foreign key (`player_id`) references players(`id`),
	unique (`player_id`, `item_id`, `color_id`)
)
-- query:mgstpp.mgo_item_purchases.create
create table if not exists `mgo_item_purchases`
(
	id						integer	primary key autoincrement,
	player_id				bigint unsigned	not null,
	purchase_id				int unsigned	not null,
	foreign key (`player_id`) references players(`id`)
)
-- query:mgstpp.shop_purchases.create
create table if not exists `shop_purchases`
(
	id						integer	primary key autoincrement,
	player_id				bigint unsigned	not null,
	date					datetime default current_timestamp not null,
	expire_date				datetime default current_timestamp not null,
	event_type				int unsigned not null default 0,
	item_quantity			int unsigned not null default 0,
	item_type				int unsigned not null default 0,
	param1					int unsigned not null default 0,
	remaining_coin			int unsigned not null default 0,
	coin_quantity			int unsigned not null default 0,
	foreign key (`player_id`) references players(`id`)
)
-- query:mgstpp.combat_deployments.create
create table if not exists `combat_deployments`
(
	id						integer	primary key autoincrement,
	player_id				bigint unsigned	not null,
	mission_id				int unsigned not null,
	deployment_info			blob default null,
	is_win					boolean not null,
	start_date				datetime not null,
	end_date				datetime not null,
	foreign key (`player_id`) references players(`id`),
	unique (`player_id`, `mission_id`)
)
-- query:mgstpp.variables.create
create table if not exists `variables`
(
	id						integer	primary key autoincrement,
	variable_name			varchar(256) 	not null	unique,
	variable_value			json			not null
)
-- query:mgstpp.pf_leagues.create
create table if not exists `pf_leagues`
(
	id						integer	primary key autoincrement,
	type					int unsigned not null default 0,
	state					int unsigned not null default 0,
	start_date				datetime not null,
	end_date				datetime not null
)
-- query:mgstpp.pf_brackets.create
create table if not exists `pf_brackets`
(
	id						integer	primary key autoincrement,
	league_id				bigint unsigned	not null,
	state					int unsigned	not null default 0,
	foreign key (`league_id`) references pf_leagues(`id`)
)
-- query:mgstpp.pf_competitors.create
create table if not exists `pf_competitors`
(
	id						integer	primary key autoincrement,
	league_id				bigint unsigned	not null,
	bracket_id				bigint unsigned	not null,
	player_id				bigint unsigned	not null,
	bracket_rank			int unsigned not null default 0,
	bracket_rank_prev		int unsigned not null default 0,
	victory_points			int unsigned not null default 0,
	win						int unsigned not null default 0,
	narrow_win				int unsigned not null default 0,
	attack_win				int unsigned not null default 0,
	defense_win				int unsigned not null default 0,
	lose					int unsigned not null default 0,
	narrow_lose				int unsigned not null default 0,
	attack_lose				int unsigned not null default 0,
	defense_lose				int unsigned not null default 0,
	foreign key (`league_id`) references pf_leagues(`id`),
	foreign key (`bracket_id`) references pf_brackets(`id`),
	foreign key (`player_id`) references players(`id`)
)
-- query:mgstpp.pf_battles.create
create table if not exists `pf_battles`
(
	id						integer	primary key autoincrement,
	league_id				bigint unsigned	not null,
	bracket_id				bigint unsigned	not null,
	section					int unsigned	not null,
	attacker_id				bigint unsigned	not null,
	defender_id				bigint unsigned	not null,
	attacker_points			int not null default 0,
	defender_points			int not null default 0,
	attacker_buff			int unsigned not null default 0,
	defender_buff			int unsigned not null default 0,
	attacker_level			int unsigned not null default 0,
	defender_level			int unsigned not null default 0,
	attacker_capability		int unsigned not null default 0,
	defender_capability		int unsigned not null default 0,
	attacker_durability		int unsigned not null default 0,
	defender_durability		int unsigned not null default 0,
	attacker_grade			int unsigned not null default 0,
	defender_security		int unsigned not null default 0,
	attacker_staff			int unsigned not null default 0,
	defender_staff			int unsigned not null default 0,
	attacker_nuclear		int unsigned not null default 0,
	defender_nuclear		int unsigned not null default 0,
	winner_state			int unsigned not null default 0,
	date					datetime not null,
	foreign key (`bracket_id`) references pf_brackets(`id`),
	foreign key (`attacker_id`) references players(`id`),
	foreign key (`defender_id`) references players(`id`)
)
-- query:mgstpp.pf_applications.create
create table if not exists `pf_applications`
(
	id						integer	primary key autoincrement,
	player_id				bigint unsigned	not null,
	league_id				bigint unsigned	default null,
	read_state				int unsigned	not null default 0,
	date					datetime not null,
	foreign key (`player_id`) references players(`id`),
	foreign key (`league_id`) references pf_leagues(`id`)
)
-- query:mgstpp.steam_users.create
create table if not exists `steam_users`
(
	id						integer	primary key autoincrement,
	account_id				bigint unsigned	not null	 unique,
	auth_token				char(32)		default null unique,
	expire_date				datetime not null
)
