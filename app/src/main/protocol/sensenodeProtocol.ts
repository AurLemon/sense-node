import type { SenseNodeFrame } from '../../shared/types/sensenode'

const validFrameTypes = new Set([
	'sensor_frame',
	'warmup_info',
	'system_status',
	'error',
])

export function parseSenseNodeLine(line: string): SenseNodeFrame {
	const trimmed = line.trim()
	if (!trimmed) {
		throw new Error('empty line')
	}

	const value: unknown = JSON.parse(trimmed)
	if (!value || typeof value !== 'object' || Array.isArray(value)) {
		throw new Error('line is not a JSON object')
	}

	const frame = value as SenseNodeFrame
	if (!validFrameTypes.has(frame.type)) {
		throw new Error(`unsupported frame type: ${String(frame.type)}`)
	}

	return {
		...frame,
		timestamp: frame.timestamp ?? Date.now(),
	}
}
