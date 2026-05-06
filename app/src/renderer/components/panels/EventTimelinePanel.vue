<script setup lang="ts">
import { useI18n } from '../../lib/i18n'
import { useDeviceStore } from '../../stores/deviceStore'

const device = useDeviceStore()
const { t } = useI18n()
</script>

<template>
	<div class="panel table-wrap">
		<table>
			<thead>
				<tr>
					<th>{{ t('events.time') }}</th>
					<th>{{ t('events.source') }}</th>
					<th>{{ t('events.raw') }}</th>
					<th>{{ t('events.stable') }}</th>
					<th>{{ t('events.confidence') }}</th>
					<th>{{ t('events.tof') }}</th>
				</tr>
			</thead>
			<tbody>
				<tr v-for="event in device.events" :key="event.id">
					<td class="mono">
						{{ new Date(event.timestamp).toLocaleTimeString() }}
					</td>
					<td>{{ event.source }}</td>
					<td>{{ event.rawEvent }}</td>
					<td>{{ event.stableEvent }}</td>
					<td class="mono">{{ event.confidence?.toFixed(2) ?? '-' }}</td>
					<td class="mono">{{ event.tofMm ?? '-' }}</td>
				</tr>
			</tbody>
		</table>
		<div v-if="device.events.length === 0" class="empty">
			{{ t('device.notConnected') }}
		</div>
	</div>
</template>

<style scoped>
.table-wrap {
	height: 100%;
	overflow: auto;
}

table {
	width: 100%;
	border-collapse: collapse;
	font-size: 13px;
}

th,
td {
	border-bottom: 1px solid var(--color-border);
	padding: 10px;
	text-align: left;
}

th {
	position: sticky;
	top: 0;
	background: var(--color-bg-alt);
	color: var(--color-text-muted);
	font-weight: 500;
}

.empty {
	padding: 16px;
	color: var(--color-text-muted);
	font-size: 13px;
}
</style>
