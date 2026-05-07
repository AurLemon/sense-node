import { app } from 'electron'
import fs from 'node:fs'
import path from 'node:path'
import Database from 'better-sqlite3'
import type { EventTask } from '../../shared/types/sensenode'

function ensureDir(filePath: string): void {
	fs.mkdirSync(path.dirname(filePath), { recursive: true })
}

export class EventTaskStore {
	private readonly db: Database.Database

	constructor() {
		const dbPath = path.join(app.getPath('userData'), 'event-tasks.sqlite3')
		ensureDir(dbPath)
		this.db = new Database(dbPath)
		this.db.pragma('journal_mode = WAL')
		this.db.exec(`
			create table if not exists event_tasks (
				id text primary key,
				name text not null,
				enabled integer not null,
				event text not null,
				cooldownMs integer not null,
				actionType text not null,
				title text not null,
				message text not null,
				command text,
				arguments text,
				createdAt integer not null,
				updatedAt integer not null
			)
		`)
	}

	list(): EventTask[] {
		const rows = this.db
			.prepare('select * from event_tasks order by updatedAt desc')
			.all() as Array<Record<string, unknown>>
		return rows.map(mapRow)
	}

	upsert(task: EventTask): EventTask {
		const row = toRow(task)
		this.db
			.prepare(
				`insert into event_tasks
				(id, name, enabled, event, cooldownMs, actionType, title, message, command, arguments, createdAt, updatedAt)
				values (@id, @name, @enabled, @event, @cooldownMs, @actionType, @title, @message, @command, @arguments, @createdAt, @updatedAt)
				on conflict(id) do update set
				name=excluded.name,
				enabled=excluded.enabled,
				event=excluded.event,
				cooldownMs=excluded.cooldownMs,
				actionType=excluded.actionType,
				title=excluded.title,
				message=excluded.message,
				command=excluded.command,
				arguments=excluded.arguments,
				updatedAt=excluded.updatedAt`,
			)
			.run(row)
		return task
	}

	remove(id: string): void {
		this.db.prepare('delete from event_tasks where id = ?').run(id)
	}

	findByEvent(event: string): EventTask[] {
		const rows = this.db
			.prepare('select * from event_tasks where enabled = 1 and event = ?')
			.all(event) as Array<Record<string, unknown>>
		return rows.map(mapRow)
	}
}

function toRow(task: EventTask): Record<string, unknown> {
	return {
		...task,
		enabled: task.enabled ? 1 : 0,
		actionType: task.action.type,
		title: task.action.title,
		message: task.action.message,
		command: task.action.command ?? null,
		arguments: JSON.stringify(task.action.arguments ?? []),
	}
}

function mapRow(row: Record<string, unknown>): EventTask {
	return {
		id: String(row.id),
		name: String(row.name),
		enabled: Number(row.enabled) === 1,
		event: row.event as EventTask['event'],
		cooldownMs: Number(row.cooldownMs),
		action: {
			type: row.actionType as EventTask['action']['type'],
			title: String(row.title),
			message: String(row.message),
			command: row.command ? String(row.command) : undefined,
			arguments: JSON.parse(String(row.arguments ?? '[]')),
		},
		createdAt: Number(row.createdAt),
		updatedAt: Number(row.updatedAt),
	}
}
