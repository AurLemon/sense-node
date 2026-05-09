<script setup lang="ts">
import { computed } from 'vue'
import SerialConnectionBar from '../controls/SerialConnectionBar.vue'
import DeviceTelemetryChart from './DeviceTelemetryChart.vue'
import EventLogPanel from './EventLogPanel.vue'
import EventTimelinePanel from './EventTimelinePanel.vue'
import MetricGrid from './MetricGrid.vue'
import SerialMonitorPanel from './SerialMonitorPanel.vue'
import { useI18n } from '../../lib/i18n'
import { useDeviceStore } from '../../stores/deviceStore'
import { useSerialStore } from '../../stores/serialStore'

const device = useDeviceStore()
const serial = useSerialStore()
const { t } = useI18n()
const isConnected = computed(() => serial.status.state === 'connected')
const eventModalUi = {
	overlay: 'fixed inset-0 z-[120] bg-default/65',
	content: 'z-[130] max-w-5xl max-h-[calc(100dvh-8rem)] overflow-hidden',
	body: 'h-[min(62vh,calc(100dvh-13rem))] min-h-0',
}
const serialModalUi = {
	overlay: 'fixed inset-0 z-[120] bg-default/65',
	content: 'z-[130] max-w-4xl max-h-[calc(100dvh-8rem)] overflow-hidden',
	body: 'h-[min(62vh,calc(100dvh-13rem))] min-h-0',
}
const logModalUi = {
	overlay: 'fixed inset-0 z-[120] bg-default/65',
	content: 'z-[130] max-w-6xl max-h-[calc(100dvh-8rem)] overflow-hidden',
	body: 'h-[min(66vh,calc(100dvh-13rem))] min-h-0',
}

function connectedValue(
	value: string | number | null | undefined,
): string | number | null | undefined {
	return isConnected.value ? value : null
}

function withUnit(
	value: string | number | null | undefined,
	unit: string,
): string | null {
	if (
		!isConnected.value ||
		value === null ||
		value === undefined ||
		value === ''
	) {
		return null
	}

	return `${value} ${unit}`
}

const metrics = computed(() => [
	{
		label: t('device.serial'),
		value: connectedValue(t(`serial.status.${serial.status.state}`)),
		icon: 'lucide:plug-zap',
	},
	{
		label: t('device.selectedPort'),
		value: connectedValue(serial.status.selectedPort ?? serial.selectedPort),
		icon: 'lucide:usb',
	},
	{
		label: t('device.baudRate'),
		value: withUnit(serial.baudRate, 'bps'),
		icon: 'lucide:gauge',
	},
	{
		label: t('device.fps'),
		value: withUnit(serial.status.fps, 'FPS'),
		icon: 'lucide:timer',
	},
	{
		label: t('device.inferenceMs'),
		value: withUnit(device.currentFrame?.inference_ms, 'ms'),
		icon: 'lucide:zap',
	},
	{
		label: t('device.freeHeap'),
		value: withUnit(device.currentFrame?.free_heap, 'B'),
		icon: 'lucide:memory-stick',
	},
	{
		label: t('device.uptimeMs'),
		value: withUnit(device.currentFrame?.uptime_ms, 'ms'),
		icon: 'lucide:clock-3',
	},
	{
		label: t('device.parseErrors'),
		value: withUnit(serial.status.parseErrorCount, t('unit.count')),
		icon: 'lucide:circle-alert',
	},
	{
		label: t('device.reconnects'),
		value: withUnit(serial.status.reconnectCount, t('unit.count')),
		icon: 'lucide:refresh-cw',
	},
	{
		label: t('device.source'),
		value: connectedValue(device.source),
		icon: 'lucide:database',
	},
	{
		label: t('device.rawEvent'),
		value: connectedValue(device.currentFrame ? device.rawEvent : null),
		icon: 'lucide:audio-lines',
	},
	{
		label: t('device.stableEvent'),
		value: connectedValue(device.currentFrame ? device.stableEvent : null),
		icon: 'lucide:badge-check',
	},
	{
		label: t('device.stabilizerState'),
		value: connectedValue(device.stabilizerSnapshot.state),
		icon: 'lucide:shield-check',
	},
	{
		label: t('device.pending'),
		value: connectedValue(
			device.currentFrame
				? (device.stabilizerSnapshot.pendingEvent ?? '-')
				: '-',
		),
		icon: 'lucide:loader-circle',
	},
	{
		label: t('device.reason'),
		value: connectedValue(
			device.currentFrame ? device.stabilizerSnapshot.reason : '-',
		),
		icon: 'lucide:file-search',
	},
])
</script>

<template>
	<div class="grid gap-3">
		<div class="flex items-center justify-between gap-3">
			<div class="min-w-0 flex-1">
				<SerialConnectionBar />
			</div>
			<div class="flex items-center gap-1.5">
				<UModal
					:title="t('events.logTitle')"
					:overlay="true"
					:portal="true"
					:ui="logModalUi"
				>
					<UTooltip :text="t('events.logTitle')">
						<UButton
							icon="i-lucide-database"
							color="neutral"
							variant="ghost"
							size="sm"
							type="button"
							:aria-label="t('events.logTitle')"
						/>
					</UTooltip>

					<template #body>
						<EventLogPanel />
					</template>
				</UModal>
				<UModal
					:title="t('tab.events')"
					:overlay="true"
					:portal="true"
					:ui="eventModalUi"
				>
					<UTooltip :text="t('tab.events')">
						<UButton
							icon="i-lucide-list-tree"
							color="neutral"
							variant="ghost"
							size="sm"
							type="button"
							:aria-label="t('tab.events')"
						/>
					</UTooltip>

					<template #body>
						<EventTimelinePanel />
					</template>
				</UModal>
				<UModal
					:title="t('tab.serial')"
					:overlay="true"
					:portal="true"
					:ui="serialModalUi"
				>
					<UTooltip :text="t('tab.serial')">
						<UButton
							icon="i-lucide-radio"
							color="neutral"
							variant="ghost"
							size="sm"
							type="button"
							:aria-label="t('tab.serial')"
						/>
					</UTooltip>

					<template #body>
						<SerialMonitorPanel />
					</template>
				</UModal>
			</div>
		</div>
		<DeviceTelemetryChart />
		<MetricGrid :items="metrics" :columns="2" />
	</div>
</template>
