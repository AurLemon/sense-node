import { autoDetect } from '@serialport/bindings-cpp'
import type { SerialPortInfo } from '../../shared/types/sensenode'

const SerialPortBinding = autoDetect()

const usbPatterns = [
	/usb/i,
	/vid[:&]/i,
	/ser=/i,
	/ch340/i,
	/cp210/i,
	/silicon labs/i,
	/wch/i,
	/ftdi/i,
	/arduino/i,
	/ttyusb/i,
]

const bluetoothPatterns = [/bthenum/i, /bluetooth/i, /蓝牙/i, /rfcomm/i]

function readPortNumber(path: string): number {
	const match = path.match(/\d+/)
	return match ? Number(match[0]) : Number.MAX_SAFE_INTEGER
}

export async function listSerialPorts(): Promise<SerialPortInfo[]> {
	const ports = await SerialPortBinding.list()

	return ports
		.map((port) => {
			const portWithFriendlyName = port as typeof port & {
				friendlyName?: string
			}
			const extendedPort = port as typeof port & {
				description?: string
				locationId?: string
				pnpId?: string
			}
			const haystack = [
				port.path,
				port.manufacturer,
				portWithFriendlyName.friendlyName,
				extendedPort.description,
				extendedPort.locationId,
				extendedPort.pnpId,
				port.vendorId,
				port.productId,
			]
				.filter(Boolean)
				.join(' ')

			const isBluetooth = bluetoothPatterns.some((pattern) =>
				pattern.test(haystack),
			)
			const isUsb =
				!isBluetooth && usbPatterns.some((pattern) => pattern.test(haystack))
			const transport: SerialPortInfo['transport'] = isBluetooth
				? 'bluetooth'
				: isUsb
					? 'usb'
					: 'other'

			return {
				path: port.path,
				manufacturer: port.manufacturer,
				serialNumber: port.serialNumber,
				vendorId: port.vendorId,
				productId: port.productId,
				friendlyName: portWithFriendlyName.friendlyName,
				transport,
				isLikelyDevice: transport === 'usb',
				portNumber: readPortNumber(port.path),
			}
		})
		.sort((a, b) => {
			if (a.transport !== b.transport) {
				if (a.transport === 'usb') return -1
				if (b.transport === 'usb') return 1
				if (a.transport === 'other' && b.transport === 'bluetooth') return -1
				if (a.transport === 'bluetooth' && b.transport === 'other') return 1
			}

			if (a.portNumber !== b.portNumber) {
				return a.portNumber - b.portNumber
			}

			return a.path.localeCompare(b.path)
		})
}
