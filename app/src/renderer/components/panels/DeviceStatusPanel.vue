<script setup lang="ts">
import { computed } from 'vue'
import SerialConnectionBar from '../controls/SerialConnectionBar.vue'
import MetricGrid from './MetricGrid.vue'
import { useI18n } from '../../lib/i18n'
import { useDeviceStore } from '../../stores/deviceStore'
import { useSerialStore } from '../../stores/serialStore'

const device = useDeviceStore()
const serial = useSerialStore()
const { t } = useI18n()

const metrics = computed(() => [
	{
		label: t('device.serial'),
		value: t(`serial.status.${serial.status.state}`),
	},
	{
		label: t('device.selectedPort'),
		value: serial.status.selectedPort ?? serial.selectedPort,
	},
	{ label: t('device.baudRate'), value: serial.baudRate },
	{ label: t('device.fps'), value: serial.status.fps },
	{ label: t('device.inferenceMs'), value: device.currentFrame?.inference_ms },
	{ label: t('device.freeHeap'), value: device.currentFrame?.free_heap },
	{ label: t('device.uptimeMs'), value: device.currentFrame?.uptime_ms },
	{ label: t('device.parseErrors'), value: serial.status.parseErrorCount },
	{ label: t('device.reconnects'), value: serial.status.reconnectCount },
	{ label: t('device.source'), value: device.source },
	{
		label: t('device.rawEvent'),
		value: device.currentFrame ? device.rawEvent : null,
	},
	{
		label: t('device.stableEvent'),
		value: device.currentFrame ? device.stableEvent : null,
	},
])
</script>

<template>
	<div class="grid gap-3">
		<SerialConnectionBar />
		<div
			v-if="!device.currentFrame"
			class="rounded-lg border border-default bg-default/75 p-3 text-muted shadow-sm backdrop-blur"
		>
			{{ t('device.notConnected') }}
		</div>
		<MetricGrid :items="metrics" />
		<div
			class="rounded-lg border border-default bg-default/75 p-3 leading-8 text-highlighted shadow-sm backdrop-blur"
		>
			<div>
				{{ t('device.stabilizerState') }}: {{ device.stabilizerSnapshot.state }}
			</div>
			<div>
				{{ t('device.pending') }}:
				{{
					device.currentFrame
						? (device.stabilizerSnapshot.pendingEvent ?? '-')
						: '-'
				}}
			</div>
			<div>
				{{ t('device.reason') }}:
				{{ device.currentFrame ? device.stabilizerSnapshot.reason : '-' }}
			</div>
		</div>
	</div>
</template>
