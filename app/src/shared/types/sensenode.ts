export type DataSourceType = 'serial' | 'wifi'

export type DataSourceConnectionState =
	| 'idle'
	| 'scanning'
	| 'connecting'
	| 'connected'
	| 'disconnecting'
	| 'disconnected'
	| 'reconnecting'
	| 'error'

export interface DataSourceStatus {
	type: DataSourceType
	state: DataSourceConnectionState
	message?: string
	selectedPort?: string
	baudRate?: number
	parseErrorCount: number
	reconnectCount: number
	fps: number
	paused?: boolean
}

export interface SerialPortInfo {
	path: string
	manufacturer?: string
	serialNumber?: string
	vendorId?: string
	productId?: string
	friendlyName?: string
	transport?: 'usb' | 'bluetooth' | 'other'
	isLikelyDevice: boolean
}

export type SenseNodeFrameType =
	| 'sensor_frame'
	| 'warmup_info'
	| 'system_status'
	| 'error'

export interface Vec3 {
	x: number
	y: number
	z: number
}

export interface SenseNodeFrame {
	type: SenseNodeFrameType
	tof_mm?: number
	accel?: Vec3
	gyro?: Vec3
	pitch?: number
	roll?: number
	yaw?: number
	imu_label?: string
	confidence?: number
	hand_state?: string
	motion_event?: string
	fusion_state?: string
	final_event?: string
	display_face?: string
	inference_ms?: number
	free_heap?: number
	uptime_ms?: number
	timestamp?: number
	sample_rate_hz?: number
	window_frames?: number
	model_labels?: string[]
	message?: string
}

export type StableEvent =
	| 'idle'
	| 'tap'
	| 'board_motion'
	| 'hand_hover'
	| 'hand_near'
	| 'hand_leave'
	| 'reject'
	| 'unknown'

export const stableEvents: StableEvent[] = [
	'idle',
	'tap',
	'board_motion',
	'hand_hover',
	'hand_near',
	'hand_leave',
	'reject',
	'unknown',
]

export type EventActionType = 'notify' | 'run_exe' | 'run_script'

export interface EventActionConfig {
	type: EventActionType
	title: string
	message: string
	command?: string
	arguments?: string[]
}

export interface EventTask {
	id: string
	name: string
	enabled: boolean
	event: StableEvent
	cooldownMs: number
	action: EventActionConfig
	createdAt: number
	updatedAt: number
}

export interface EventExecutionLog {
	id: string
	taskId: string
	event: StableEvent
	createdAt: number
	result: 'skipped' | 'success' | 'failed'
	message: string
}

export interface StabilizerSnapshot {
	rawEvent: StableEvent
	stableEvent: StableEvent
	pendingEvent: StableEvent | null
	confidence: number
	state: 'stable' | 'pending' | 'cooldown' | 'pulse'
	lastTransitionAt: number
	holdUntil: number
	reason: string
}

export interface SenseNodeEventLog {
	id: string
	timestamp: number
	source: DataSourceType
	rawEvent: string
	stableEvent: string
	confidence?: number
	tofMm?: number
	inferenceMs?: number
	frame: SenseNodeFrame
}

export interface SerialLineEntry {
	id: string
	timestamp: number
	level: 'info' | 'warning' | 'error'
	line: string
	message?: string
}

export interface SenseNodeDataSource {
	type: DataSourceType
	connect(): Promise<void>
	disconnect(): Promise<void>
	onFrame(callback: (frame: SenseNodeFrame) => void): void
	onStatus(callback: (status: DataSourceStatus) => void): void
}
