import { app } from 'electron'
import fs from 'node:fs'
import path from 'node:path'
import Database from 'better-sqlite3'
import type { SenseNodeEventLog } from '../../shared/types/sensenode'

function ensureDir(filePath: string): void {
	fs.mkdirSync(path.dirname(filePath), { recursive: true })
}

export class EventLogStore {
	private readonly db: Database.Database

	constructor() {
		const dbPath = path.join(app.getPath('userData'), 'event-logs.sqlite3')
		ensureDir(dbPath)
		this.db = new Database(dbPath)
		this.db.pragma('journal_mode = WAL')
		this.db.exec(`
			create table if not exists event_logs (
				id text primary key,
				timestamp integer not null,
				source text not null,
				rawEvent text not null,
				stableEvent text not null,
				confidence real,
				tofMm integer,
				inferenceMs real,
				imuLabel text,
				handState text,
				motionEvent text,
				fusionState text,
				finalEvent text,
				ax real,
				ay real,
				az real,
				gx real,
				gy real,
				gz real,
				frameJson text not null
			)
		`)
		this.db.exec(`
			create index if not exists event_logs_timestamp_idx
			on event_logs (timestamp desc)
		`)
	}

	add(event: SenseNodeEventLog): SenseNodeEventLog {
		this.db
			.prepare(
				`insert or replace into event_logs
				(id, timestamp, source, rawEvent, stableEvent, confidence, tofMm, inferenceMs,
				imuLabel, handState, motionEvent, fusionState, finalEvent,
				ax, ay, az, gx, gy, gz, frameJson)
				values
				(@id, @timestamp, @source, @rawEvent, @stableEvent, @confidence, @tofMm, @inferenceMs,
				@imuLabel, @handState, @motionEvent, @fusionState, @finalEvent,
				@ax, @ay, @az, @gx, @gy, @gz, @frameJson)`,
			)
			.run(toRow(event))
		return event
	}

	list(limit = 200): SenseNodeEventLog[] {
		const rows = this.db
			.prepare('select * from event_logs order by timestamp desc limit ?')
			.all(limit) as Array<Record<string, unknown>>
		return rows.map(mapRow)
	}
}

function toRow(event: SenseNodeEventLog): Record<string, unknown> {
	return {
		id: event.id,
		timestamp: event.timestamp,
		source: event.source,
		rawEvent: event.rawEvent,
		stableEvent: event.stableEvent,
		confidence: event.confidence ?? null,
		tofMm: event.tofMm ?? null,
		inferenceMs: event.inferenceMs ?? null,
		imuLabel: event.frame.imu_label ?? null,
		handState: event.frame.hand_state ?? null,
		motionEvent: event.frame.motion_event ?? null,
		fusionState: event.frame.fusion_state ?? null,
		finalEvent: event.frame.final_event ?? null,
		ax: event.frame.accel?.x ?? null,
		ay: event.frame.accel?.y ?? null,
		az: event.frame.accel?.z ?? null,
		gx: event.frame.gyro?.x ?? null,
		gy: event.frame.gyro?.y ?? null,
		gz: event.frame.gyro?.z ?? null,
		frameJson: JSON.stringify(event.frame),
	}
}

function mapRow(row: Record<string, unknown>): SenseNodeEventLog {
	const frame = JSON.parse(String(row.frameJson)) as SenseNodeEventLog['frame']
	return {
		id: String(row.id),
		timestamp: Number(row.timestamp),
		source: row.source as SenseNodeEventLog['source'],
		rawEvent: String(row.rawEvent),
		stableEvent: String(row.stableEvent),
		confidence: optionalNumber(row.confidence),
		tofMm: optionalNumber(row.tofMm),
		inferenceMs: optionalNumber(row.inferenceMs),
		frame,
	}
}

function optionalNumber(value: unknown): number | undefined {
	if (value === null || value === undefined) {
		return undefined
	}
	return Number(value)
}
