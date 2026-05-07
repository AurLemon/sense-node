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

	if (trimmed.includes('{')) {
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

	if (/^-?\d+(?:\.\d+)?,/.test(trimmed)) {
		const parts = trimmed.split(',').map((part) => part.trim())
		if (parts.length !== 7) {
			throw new Error('csv frame does not have 7 values')
		}

		const [tof, ax, ay, az, gx, gy, gz] = parts.map(Number)
		if ([tof, ax, ay, az, gx, gy, gz].some((value) => Number.isNaN(value))) {
			throw new Error('csv frame has invalid numeric values')
		}

		return {
			type: 'sensor_frame',
			tof_mm: tof,
			accel: { x: ax, y: ay, z: az },
			gyro: { x: gx, y: gy, z: gz },
			timestamp: Date.now(),
		}
	}

	throw new Error('line is not a supported payload')
}
