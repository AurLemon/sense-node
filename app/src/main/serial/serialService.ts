import { EventEmitter } from 'node:events'
import { autoDetect } from '@serialport/bindings-cpp'
import { ReadlineParser } from '@serialport/parser-readline'
import { SerialPortStream } from '@serialport/stream'
import { parseSenseNodeLine } from '../protocol/sensenodeProtocol'
import type {
	DataSourceStatus,
	SerialLineEntry,
} from '../../shared/types/sensenode'

const SerialPortBinding = autoDetect()
type SerialPort = SerialPortStream<typeof SerialPortBinding>

export class SerialService extends EventEmitter {
	private port: SerialPort | null = null
	private selectedPort?: string
	private baudRate = 115200
	private reconnectTimer: NodeJS.Timeout | null = null
	private parseErrorCount = 0
	private reconnectCount = 0
	private frameCount = 0
	private lastFpsAt = Date.now()
	private fps = 0
	private paused = false
	private manuallyDisconnected = false

	getStatus(
		state: DataSourceStatus['state'],
		message?: string,
	): DataSourceStatus {
		return {
			type: 'serial',
			state,
			message,
			selectedPort: this.selectedPort,
			baudRate: this.baudRate,
			parseErrorCount: this.parseErrorCount,
			reconnectCount: this.reconnectCount,
			fps: this.fps,
			paused: this.paused,
		}
	}

	async connect(path: string, baudRate = 115200): Promise<void> {
		await this.disconnect(false)
		this.selectedPort = path
		this.baudRate = baudRate
		this.manuallyDisconnected = false
		this.emit('status', this.getStatus('connecting'))

		this.port = new SerialPortStream({
			binding: SerialPortBinding,
			path,
			baudRate,
			autoOpen: false,
		})
		const parser = this.port.pipe(new ReadlineParser({ delimiter: '\n' }))

		parser.on('data', (line: string) => this.handleLine(line))
		this.port.on('close', () => this.handleClose())
		this.port.on('error', (error) => {
			this.emitLine('error', '', error.message)
			this.emit('status', this.getStatus('error', error.message))
		})

		await new Promise<void>((resolve, reject) => {
			this.port?.open((error) => {
				if (error) {
					reject(error)
					return
				}
				resolve()
			})
		})

		this.emit('status', this.getStatus('connected'))
	}

	async disconnect(markManual = true): Promise<void> {
		if (markManual) {
			this.manuallyDisconnected = true
		}
		this.clearReconnectTimer()
		const currentPort = this.port
		this.port = null

		if (!currentPort?.isOpen) {
			this.emit('status', this.getStatus('disconnected'))
			return
		}

		this.emit('status', this.getStatus('disconnecting'))
		await new Promise<void>((resolve) => {
			currentPort.close(() => resolve())
		})
		this.emit('status', this.getStatus('disconnected'))
	}

	pause(): void {
		this.paused = true
		this.emit(
			'status',
			this.getStatus(this.port?.isOpen ? 'connected' : 'disconnected'),
		)
	}

	resume(): void {
		this.paused = false
		this.emit(
			'status',
			this.getStatus(this.port?.isOpen ? 'connected' : 'disconnected'),
		)
	}

	private handleLine(line: string): void {
		this.emitLine('info', line)
		if (this.paused) {
			return
		}

		try {
			const frame = parseSenseNodeLine(line)
			this.updateFps()
			this.emit('frame', frame)
		} catch (error) {
			this.parseErrorCount += 1
			this.emitLine(
				'warning',
				line,
				error instanceof Error ? error.message : 'parse failed',
			)
			this.emit('status', this.getStatus('connected'))
		}
	}

	private handleClose(): void {
		if (this.manuallyDisconnected) {
			return
		}
		this.emit('status', this.getStatus('disconnected', 'serial port closed'))
		if (!this.selectedPort) {
			return
		}
		this.reconnectCount += 1
		this.emit('status', this.getStatus('reconnecting'))
		this.clearReconnectTimer()
		this.reconnectTimer = setTimeout(() => {
			if (this.selectedPort) {
				void this.connect(this.selectedPort, this.baudRate).catch((error) => {
					this.emitLine(
						'error',
						'',
						error instanceof Error ? error.message : String(error),
					)
					this.handleClose()
				})
			}
		}, 1500)
	}

	private clearReconnectTimer(): void {
		if (this.reconnectTimer) {
			clearTimeout(this.reconnectTimer)
			this.reconnectTimer = null
		}
	}

	private updateFps(): void {
		this.frameCount += 1
		const now = Date.now()
		const elapsed = now - this.lastFpsAt
		if (elapsed >= 1000) {
			this.fps = Math.round((this.frameCount * 1000) / elapsed)
			this.frameCount = 0
			this.lastFpsAt = now
			this.emit('status', this.getStatus('connected'))
		}
	}

	private emitLine(
		level: SerialLineEntry['level'],
		line: string,
		message?: string,
	): void {
		this.emit('line', {
			id: `${Date.now()}-${Math.random().toString(16).slice(2)}`,
			timestamp: Date.now(),
			level,
			line,
			message,
		})
	}
}
