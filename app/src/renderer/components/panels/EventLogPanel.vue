<script setup lang="ts">
import { onMounted } from 'vue'
import { useI18n } from '../../lib/i18n'
import { useDeviceStore } from '../../stores/deviceStore'
import type { SenseNodeEventLog } from '../../../shared/types/sensenode'

const device = useDeviceStore()
const { t } = useI18n()

onMounted(() => {
	void device.loadPersistedEvents()
})

function formatNumber(value?: number, digits = 2): string {
	if (value === undefined || Number.isNaN(value)) {
		return '-'
	}
	return value.toFixed(digits)
}

function formatTofValue(value?: number): string {
	if (value === undefined || value < 0 || value === 8191) {
		return '-'
	}
	return String(value)
}

function formatVec3(value?: { x: number; y: number; z: number }): string {
	if (!value) {
		return '-'
	}
	return `${formatNumber(value.x)} / ${formatNumber(value.y)} / ${formatNumber(value.z)}`
}

function eventLabel(event?: string): string {
	if (!event) {
		return '-'
	}
	return t(`events.event.${event}`)
}

function rawFrame(log: SenseNodeEventLog): string {
	return JSON.stringify(log.frame)
}
</script>

<template>
	<div class="flex h-full min-h-0 flex-col overflow-hidden">
		<div
			class="flex items-center justify-between gap-3 border-b border-default pb-3"
		>
			<div class="text-sm text-muted">
				{{ t('events.logHint') }}
			</div>
			<UButton
				icon="i-lucide-refresh-cw"
				color="neutral"
				variant="ghost"
				size="sm"
				type="button"
				:label="t('common.refresh')"
				@click="device.loadPersistedEvents()"
			/>
		</div>

		<UScrollArea class="min-h-0 flex-1">
			<table class="w-full min-w-[980px] border-collapse text-[13px]">
				<thead>
					<tr>
						<th
							class="sticky top-0 border-b border-default bg-default px-2.5 py-2.5 text-left font-medium text-muted"
						>
							{{ t('events.time') }}
						</th>
						<th
							class="sticky top-0 border-b border-default bg-default px-2.5 py-2.5 text-left font-medium text-muted"
						>
							{{ t('events.raw') }}
						</th>
						<th
							class="sticky top-0 border-b border-default bg-default px-2.5 py-2.5 text-left font-medium text-muted"
						>
							{{ t('events.stable') }}
						</th>
						<th
							class="sticky top-0 border-b border-default bg-default px-2.5 py-2.5 text-left font-medium text-muted"
						>
							{{ t('events.imuLabel') }}
						</th>
						<th
							class="sticky top-0 border-b border-default bg-default px-2.5 py-2.5 text-left font-medium text-muted"
						>
							{{ t('events.confidence') }}
						</th>
						<th
							class="sticky top-0 border-b border-default bg-default px-2.5 py-2.5 text-left font-medium text-muted"
						>
							{{ t('events.tof') }}
						</th>
						<th
							class="sticky top-0 border-b border-default bg-default px-2.5 py-2.5 text-left font-medium text-muted"
						>
							{{ t('events.handState') }}
						</th>
						<th
							class="sticky top-0 border-b border-default bg-default px-2.5 py-2.5 text-left font-medium text-muted"
						>
							{{ t('events.accel') }}
						</th>
						<th
							class="sticky top-0 border-b border-default bg-default px-2.5 py-2.5 text-left font-medium text-muted"
						>
							{{ t('events.gyro') }}
						</th>
						<th
							class="sticky top-0 border-b border-default bg-default px-2.5 py-2.5 text-left font-medium text-muted"
						>
							{{ t('events.rawJson') }}
						</th>
					</tr>
				</thead>
				<tbody>
					<tr v-for="event in device.persistedEvents" :key="event.id">
						<td class="border-b border-default px-2.5 py-2.5 tabular-nums">
							{{ new Date(event.timestamp).toLocaleString() }}
						</td>
						<td class="border-b border-default px-2.5 py-2.5">
							{{ eventLabel(event.rawEvent) }}
						</td>
						<td class="border-b border-default px-2.5 py-2.5">
							{{ eventLabel(event.stableEvent) }}
						</td>
						<td class="border-b border-default px-2.5 py-2.5">
							{{ event.frame.imu_label ?? '-' }}
						</td>
						<td class="border-b border-default px-2.5 py-2.5 tabular-nums">
							{{ formatNumber(event.confidence) }}
						</td>
						<td class="border-b border-default px-2.5 py-2.5 tabular-nums">
							{{ formatTofValue(event.tofMm) }}
						</td>
						<td class="border-b border-default px-2.5 py-2.5">
							{{ event.frame.hand_state ?? '-' }}
						</td>
						<td class="border-b border-default px-2.5 py-2.5 tabular-nums">
							{{ formatVec3(event.frame.accel) }}
						</td>
						<td class="border-b border-default px-2.5 py-2.5 tabular-nums">
							{{ formatVec3(event.frame.gyro) }}
						</td>
						<td
							class="max-w-[360px] truncate border-b border-default px-2.5 py-2.5 font-mono text-xs text-muted"
							:title="rawFrame(event)"
						>
							{{ rawFrame(event) }}
						</td>
					</tr>
				</tbody>
			</table>
			<div
				v-if="device.persistedEvents.length === 0"
				class="p-4 text-sm text-muted"
			>
				{{ t('events.logEmpty') }}
			</div>
		</UScrollArea>
	</div>
</template>
