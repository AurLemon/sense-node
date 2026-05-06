import { defineStore } from 'pinia'
import { ref } from 'vue'
import type {
	DataSourceStatus,
	SerialLineEntry,
	SerialPortInfo,
} from '../../shared/types/sensenode'

export const useSerialStore = defineStore('serial', () => {
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

	function pickDefaultPort(): string {
		const likelyPort = ports.value.find((port) => port.isLikelyDevice)
		return likelyPort?.path ?? ports.value[0]?.path ?? ''
	}

	async function refreshPorts(): Promise<void> {
		status.value = { ...status.value, state: 'scanning' }
		ports.value = await window.sensenode.serial.listPorts()
		const selectedExists = ports.value.some(
			(port) => port.path === selectedPort.value,
		)
		if (!selectedPort.value || !selectedExists) {
			selectedPort.value = pickDefaultPort()
		}
		status.value = { ...status.value, state: 'idle' }
	}

	async function connect(): Promise<void> {
		if (!selectedPort.value) {
			return
		}
		status.value = await window.sensenode.serial.connect(
			selectedPort.value,
			baudRate.value,
		)
	}

	async function disconnect(): Promise<void> {
		status.value = await window.sensenode.serial.disconnect()
	}

	function pushLine(line: SerialLineEntry): void {
		lines.value.unshift(line)
		if (lines.value.length > 240) {
			lines.value.length = 240
		}
	}

	return {
		ports,
		lines,
		status,
		selectedPort,
		baudRate,
		refreshPorts,
		connect,
		disconnect,
		pushLine,
	}
})
