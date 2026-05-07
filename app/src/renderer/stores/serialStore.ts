import { defineStore } from 'pinia'
import { ref } from 'vue'
import type {
	DataSourceStatus,
	SenseNodeFrame,
	SerialLineEntry,
	SerialPortInfo,
} from '../../shared/types/sensenode'

export const useSerialStore = defineStore('serial', () => {
	const lastSuccessfulPortKey = 'sensenode.serial.lastSuccessfulPort'
	const frameValidationTimeoutMs = 1000
	const ports = ref<SerialPortInfo[]>([])
	const lines = ref<SerialLineEntry[]>([])
	const status = ref<DataSourceStatus>({
		type: 'serial',
		state: 'idle',
		parseErrorCount: 0,
		reconnectCount: 0,
		fps: 0,
		baudRate: 115200,
	})
	const selectedPort = ref('')
	const baudRate = ref(115200)
	let pendingFrameResolver: (() => void) | null = null

	function pickDefaultPort(): string {
		const likelyPort = ports.value.find((port) => port.isLikelyDevice)
		return likelyPort?.path ?? ports.value[0]?.path ?? ''
	}

	function readLastSuccessfulPort(): string {
		if (typeof window === 'undefined') {
			return ''
		}

		return window.localStorage.getItem(lastSuccessfulPortKey) ?? ''
	}

	function writeLastSuccessfulPort(path: string): void {
		if (typeof window !== 'undefined') {
			window.localStorage.setItem(lastSuccessfulPortKey, path)
		}
	}

	function getAutoConnectCandidates(): string[] {
		const lastSuccessfulPort = readLastSuccessfulPort()
		const candidatePorts = new Set<string>()
		const lastPort = ports.value.find(
			(port) => port.path === lastSuccessfulPort,
		)

		if (lastPort) {
			candidatePorts.add(lastPort.path)
		}

		for (const port of ports.value) {
			if (port.transport === 'usb') {
				candidatePorts.add(port.path)
			}
		}

		return [...candidatePorts]
	}

	async function refreshPorts(): Promise<void> {
		const previousStatus = status.value
		status.value = { ...status.value, state: 'scanning' }
		ports.value = await window.sensenode.serial.listPorts()
		const selectedExists = ports.value.some(
			(port) => port.path === selectedPort.value,
		)
		if (!selectedPort.value || !selectedExists) {
			selectedPort.value = pickDefaultPort()
		}
		status.value = { ...previousStatus }
	}

	async function connect(): Promise<boolean> {
		if (!selectedPort.value) {
			return false
		}
		status.value = await window.sensenode.serial.connect(
			selectedPort.value,
			baudRate.value,
		)
		writeLastSuccessfulPort(selectedPort.value)
		return true
	}

	async function connectAndValidate(
		path = selectedPort.value,
	): Promise<boolean> {
		if (!path) {
			return false
		}

		try {
			selectedPort.value = path
			status.value = await window.sensenode.serial.connect(path, baudRate.value)
			const validFrameReceived = await waitForValidFrame(
				frameValidationTimeoutMs,
			)

			if (validFrameReceived) {
				writeLastSuccessfulPort(path)
				return true
			}
		} catch {
			// Try the next candidate; the line log already carries native errors.
		}

		status.value = await window.sensenode.serial.disconnect()
		return false
	}

	async function autoConnectUsb(): Promise<boolean> {
		await refreshPorts()
		const candidates = getAutoConnectCandidates()

		for (const path of candidates) {
			const connected = await connectAndValidate(path)
			if (connected) {
				return true
			}
		}

		return false
	}

	async function disconnect(): Promise<void> {
		status.value = await window.sensenode.serial.disconnect()
	}

	function notifyFrame(_frame: SenseNodeFrame): void {
		pendingFrameResolver?.()
		pendingFrameResolver = null
	}

	function pushLine(line: SerialLineEntry): void {
		lines.value.unshift(line)
		if (lines.value.length > 240) {
			lines.value.length = 240
		}
	}

	function waitForValidFrame(timeoutMs: number): Promise<boolean> {
		return new Promise((resolve) => {
			let resolved = false
			const timeout = window.setTimeout(() => {
				if (resolved) return
				resolved = true
				if (pendingFrameResolver === resolveFrame) {
					pendingFrameResolver = null
				}
				resolve(false)
			}, timeoutMs)

			function resolveFrame(): void {
				if (resolved) return
				resolved = true
				window.clearTimeout(timeout)
				resolve(true)
			}

			pendingFrameResolver = resolveFrame
		})
	}

	return {
		ports,
		lines,
		status,
		selectedPort,
		baudRate,
		refreshPorts,
		connect,
		connectAndValidate,
		autoConnectUsb,
		disconnect,
		notifyFrame,
		pushLine,
	}
})
