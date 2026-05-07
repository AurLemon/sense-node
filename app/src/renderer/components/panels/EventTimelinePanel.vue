<script setup lang="ts">
import { useI18n } from '../../lib/i18n'
import { useDeviceStore } from '../../stores/deviceStore'

const device = useDeviceStore()
const { t } = useI18n()

function formatTofValue(value?: number): string {
	if (value === undefined || value < 0 || value === 8191) {
		return '-'
	}

	return String(value)
}
</script>

<template>
	<div class="flex h-full min-h-0 flex-col overflow-hidden">
		<UScrollArea class="min-h-0 flex-1">
			<table class="w-full border-collapse text-[13px]">
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
							{{ t('events.source') }}
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
							{{ t('events.confidence') }}
						</th>
						<th
							class="sticky top-0 border-b border-default bg-default px-2.5 py-2.5 text-left font-medium text-muted"
						>
							{{ t('events.tof') }}
						</th>
					</tr>
				</thead>
				<tbody>
					<tr v-for="event in device.events" :key="event.id">
						<td class="border-b border-default px-2.5 py-2.5">
							{{ new Date(event.timestamp).toLocaleTimeString() }}
						</td>
						<td class="border-b border-default px-2.5 py-2.5">
							{{ event.source }}
						</td>
						<td class="border-b border-default px-2.5 py-2.5">
							{{ event.rawEvent }}
						</td>
						<td class="border-b border-default px-2.5 py-2.5">
							{{ event.stableEvent }}
						</td>
						<td class="border-b border-default px-2.5 py-2.5">
							{{ event.confidence?.toFixed(2) ?? '-' }}
						</td>
						<td class="border-b border-default px-2.5 py-2.5">
							{{ formatTofValue(event.tofMm) }}
						</td>
					</tr>
				</tbody>
			</table>
			<div v-if="device.events.length === 0" class="p-4 text-sm text-muted">
				{{ t('device.notConnected') }}
			</div>
		</UScrollArea>
	</div>
</template>
